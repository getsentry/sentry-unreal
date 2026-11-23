// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryShaderSymbols.h"

#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
#include "Interfaces/ITargetPlatform.h"
#include "UObject/ICookInfo.h"
#endif

#ifndef NV_AFTERMATH
#define NV_AFTERMATH 0
#endif

// Engines before 5.2 ship an Aftermath SDK without GFSDK_Aftermath_ShaderBinaryHash
#define SENTRY_CAN_HASH_SHADERS (PLATFORM_WINDOWS && NV_AFTERMATH && !UE_VERSION_OLDER_THAN(5, 2, 0))

#if SENTRY_CAN_HASH_SHADERS
#include "Windows/AllowWindowsPlatformTypes.h"
THIRD_PARTY_INCLUDES_START
// clang-format off
#include "d3d12.h" // declares the types that gate GFSDK_Aftermath_GetShaderHash below
#include "GFSDK_Aftermath_GpuCrashDumpDecoding.h"
// clang-format on
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformTypes.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSentryShaderSymbols, Log, All);

const TCHAR* const FSentryShaderSymbols::ManifestFileName = TEXT("sentry-shader-manifest.tsv");

void FSentryShaderSymbols::RegisterCookHook()
{
#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	UE::Cook::FDelegates::CookFinished.AddLambda([](UE::Cook::ICookInfo& CookInfo)
	{
		if (!FSentryModule::Get().GetSettings()->UploadShaderSymbols || CookInfo.GetProcessType() == UE::Cook::EProcessType::Worker)
		{
			return;
		}

		const bool bCooksWindows = CookInfo.GetSessionPlatforms().ContainsByPredicate([](const ITargetPlatform* Platform)
		{
			return Platform && Platform->IniPlatformName() == TEXT("Windows");
		});
		if (!bCooksWindows)
		{
			UE_LOG(LogSentryShaderSymbols, Log, TEXT("No Windows platform in the cook session, skipping shader symbols"));
			return;
		}

#if SENTRY_CAN_HASH_SHADERS
		Run(FPaths::ProjectSavedDir(), true);
#else
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("Shader symbols upload skipped: shader hashing needs the NVIDIA Aftermath library, which is only available on Windows editors"));
#endif
	});
#endif
}

int32 FSentryShaderSymbols::Run(const FString& ProjectSavedDir, bool bUpload)
{
#if SENTRY_CAN_HASH_SHADERS
	const FString SymbolsRoot = ProjectSavedDir / TEXT("ShaderSymbols");
	TArray<FString> FormatDirs;
	IFileManager::Get().FindFiles(FormatDirs, *(SymbolsRoot / TEXT("*")), false, true);

	TArray<FString> ProcessedDirs;
	int32 Failures = 0;
	for (const FString& FormatName : FormatDirs)
	{
		const FString FormatDir = SymbolsRoot / FormatName;
		TArray<FString> DxilFiles;
		IFileManager::Get().FindFiles(DxilFiles, *(FormatDir / TEXT("*.dxil")), true, false);
		if (DxilFiles.Num() == 0)
		{
			UE_LOG(LogSentryShaderSymbols, Log, TEXT("%s: no .dxil files, skipping (not a DXC format or r.Shaders.Symbols is off)"), *FormatName);
			continue;
		}

		DxilFiles.Sort();

		int32 ShaderCount = 0;
		const double StartSeconds = FPlatformTime::Seconds();
		if (!WriteManifest(FormatDir, DxilFiles, ShaderCount))
		{
			Failures++;
			continue;
		}
		UE_LOG(LogSentryShaderSymbols, Display, TEXT("%s: manifest with %d shaders written in %.1f s"), *FormatName, ShaderCount, FPlatformTime::Seconds() - StartSeconds);
		ProcessedDirs.Add(FormatDir);
	}

	if (ProcessedDirs.Num() == 0)
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("No shader symbols found under %s. Set r.Shaders.Symbols=1 in the [ShaderCompiler] section of the target platform's Engine.ini and cook again."), *SymbolsRoot);
	}
	else if (bUpload && Upload(ProcessedDirs) != 0)
	{
		Failures++;
	}
	return Failures == 0 ? 0 : 1;
#else
	UE_LOG(LogSentryShaderSymbols, Error, TEXT("Shader hashing needs the NVIDIA Aftermath library, which is only available on Windows editors (UE 5.2+)"));
	return 1;
#endif
}

bool FSentryShaderSymbols::WriteManifest(const FString& FormatDir, const TArray<FString>& DxilFiles, int32& OutShaderCount)
{
	const TMap<FString, FString> Groups = ReadSymbolsInfo(FormatDir);

	FString Manifest;
	int32 HashFailures = 0;
	OutShaderCount = 0;
	for (const FString& FileName : DxilFiles)
	{
		const FString FilePath = FormatDir / FileName;
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
		{
			HashFailures++;
			continue;
		}

		uint64 Hash = 0;
		if (!HashShader(Bytes, Hash))
		{
			HashFailures++;
			continue;
		}

		const FString Stem = FPaths::GetBaseFilename(FileName);
		const FString* Group = Groups.Find(Stem.ToLower());
		Manifest += FString::Printf(TEXT("%llu\t%s.pdb\t%s\t%s\n"), Hash, *Stem, *FPaths::ConvertRelativePathToFull(FilePath), Group ? **Group : TEXT(""));
		OutShaderCount++;
	}

	if (HashFailures > 0)
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("%d shader binaries could not be hashed"), HashFailures);
	}

	if (OutShaderCount == 0)
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(Manifest, *(FormatDir / ManifestFileName), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool FSentryShaderSymbols::HashShader(const TArray<uint8>& Shader, uint64& OutHash)
{
#if SENTRY_CAN_HASH_SHADERS
	static bool bDllDirectoryPushed = false;
	if (!bDllDirectoryPushed)
	{
		FPlatformProcess::PushDllDirectory(*FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/NVIDIA/NVaftermath/Win64")));
		bDllDirectoryPushed = true;
	}

	const D3D12_SHADER_BYTECODE Bytecode{ Shader.GetData(), static_cast<SIZE_T>(Shader.Num()) };
	GFSDK_Aftermath_ShaderBinaryHash Hash{};
	if (GFSDK_Aftermath_GetShaderHash(GFSDK_Aftermath_Version_API, &Bytecode, &Hash) != GFSDK_Aftermath_Result_Success)
	{
		return false;
	}
	OutHash = Hash.hash;
	return true;
#else
	return false;
#endif
}

TMap<FString, FString> FSentryShaderSymbols::ReadSymbolsInfo(const FString& FormatDir)
{
	TMap<FString, FString> Groups;
	TArray<FString> Lines;
	FFileHelper::LoadFileToStringArray(Lines, *(FormatDir / TEXT("ShaderSymbols.info")));
	for (const FString& Line : Lines)
	{
		FString Hash, Group;
		if (Line.Split(TEXT(" "), &Hash, &Group))
		{
			Groups.Add(Hash.ToLower(), Group);
		}
	}
	return Groups;
}

int32 FSentryShaderSymbols::Upload(const TArray<FString>& FormatDirs)
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Sentry"));
	if (!Plugin.IsValid())
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("Shader symbols upload skipped: Sentry plugin not found"));
		return 1;
	}

	const FString PluginDir = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	const FString PythonPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() / TEXT("Binaries/ThirdParty/Python3/Win64/python.exe"));
	const FString ScriptPath = PluginDir / TEXT("Scripts/upload_debug_symbols.py");

	FString Args = FString::Printf(TEXT("-B -X utf8 \"%s\" dxil \"%s\" \"%s\""), *ScriptPath, *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()), *PluginDir);
	for (const FString& FormatDir : FormatDirs)
	{
		Args += FString::Printf(TEXT(" \"%s\""), *FPaths::ConvertRelativePathToFull(FormatDir));
	}

	int32 ReturnCode = -1;
	FString StdOut, StdErr;
	if (!FPlatformProcess::ExecProcess(*PythonPath, *Args, &ReturnCode, &StdOut, &StdErr))
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("Failed to start %s"), *PythonPath);
		return 1;
	}

	TArray<FString> OutLines;
	StdOut.ParseIntoArrayLines(OutLines);
	const int32 TailStart = FMath::Max(0, OutLines.Num() - 8);
	for (int32 Index = 0; Index < OutLines.Num(); ++Index)
	{
		if (Index >= TailStart || OutLines[Index].StartsWith(TEXT("Sentry: ")))
		{
			UE_LOG(LogSentryShaderSymbols, Display, TEXT("  %s"), *OutLines[Index]);
		}
	}
	if (!StdErr.IsEmpty())
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("%s"), *StdErr);
	}
	UE_LOG(LogSentryShaderSymbols, Display, TEXT("Shader symbols upload finished with code %d"), ReturnCode);
	return ReturnCode;
}
