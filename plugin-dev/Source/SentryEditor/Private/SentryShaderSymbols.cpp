// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryShaderSymbols.h"

#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SentrySettings.h"
#include "SentrySymToolsDownloader.h"

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
#include "Interfaces/ITargetPlatform.h"
#include "UObject/ICookInfo.h"
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
		if (!GetDefault<USentrySettings>()->UploadShaderSymbols || CookInfo.GetProcessType() == UE::Cook::EProcessType::Worker)
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

		Run(FPaths::ProjectSavedDir(), true);
	});
#endif
}

int32 FSentryShaderSymbols::Run(const FString& ProjectSavedDir, bool bUpload)
{
#if !SENTRY_CAN_HASH_SHADERS
	UE_LOG(LogSentryShaderSymbols, Error, TEXT("Shader hashing needs the NVIDIA Aftermath library, which is only available on Windows editors (UE 5.2+)"));
	return 1;
#endif

	const FString SymbolsRoot = ProjectSavedDir / TEXT("ShaderSymbols");
	TArray<FString> FormatDirs;
	IFileManager::Get().FindFiles(FormatDirs, *(SymbolsRoot / TEXT("*")), false, true);

	int32 Processed = 0;
	int32 Failures = 0;
	if (bUpload && !ConfigureCliEnvironment())
	{
		bUpload = false;
		Failures++;
	}

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
		UE_LOG(LogSentryShaderSymbols, Log, TEXT("%s: manifest with %d shaders written in %.1f s"), *FormatName, ShaderCount, FPlatformTime::Seconds() - StartSeconds);
		Processed++;

		if (bUpload && Upload(FormatDir) != 0)
		{
			Failures++;
		}
	}

	if (Processed == 0)
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("No shader symbols found under %s. Set r.Shaders.Symbols=1 in the [ShaderCompiler] section of the target platform's Engine.ini and cook again."), *SymbolsRoot);
	}
	return Failures == 0 ? 0 : 1;
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

FString FSentryShaderSymbols::ReadProperty(const TArray<FString>& PropertyLines, const FString& Key)
{
	for (const FString& Line : PropertyLines)
	{
		const FString Trimmed = Line.TrimStartAndEnd();
		if (Trimmed.IsEmpty() || Trimmed.StartsWith(TEXT("#")) || Trimmed.StartsWith(TEXT(";")))
		{
			continue;
		}

		FString LineKey, LineValue;
		if (Trimmed.Split(TEXT("="), &LineKey, &LineValue) && LineKey.TrimEnd() == Key)
		{
			return LineValue.TrimStart();
		}
	}
	return FString();
}

bool FSentryShaderSymbols::ConfigureCliEnvironment()
{
	FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_PROPERTIES"), TEXT(""));

	const FString PropertiesFile = FPaths::ProjectDir() / TEXT("sentry.properties");
	TArray<FString> PropertyLines;
	const bool bHasPropertiesFile = FFileHelper::LoadFileToStringArray(PropertyLines, *PropertiesFile);
	if (!bHasPropertiesFile)
	{
		UE_LOG(LogSentryShaderSymbols, Log, TEXT("Properties file not found at %s, falling back to environment variables"), *PropertiesFile);
	}

	struct FCliSetting
	{
		const TCHAR* EnvName;
		const TCHAR* PropertyKey;
		bool bRequired;
	};
	const FCliSetting Settings[] = {
		{ TEXT("SENTRY_ORG"), TEXT("defaults.org"), true },
		{ TEXT("SENTRY_PROJECT"), TEXT("defaults.project"), true },
		{ TEXT("SENTRY_AUTH_TOKEN"), TEXT("auth.token"), true },
		{ TEXT("SENTRY_URL"), TEXT("defaults.url"), false },
	};

	bool bAnyFromEnv = false;
	bool bAnyFromFile = false;
	for (const FCliSetting& Setting : Settings)
	{
		const FString EnvValue = FPlatformMisc::GetEnvironmentVariable(Setting.EnvName).TrimStartAndEnd();
		if (!EnvValue.IsEmpty())
		{
			bAnyFromEnv = true;
			continue;
		}

		const FString FileValue = ReadProperty(PropertyLines, Setting.PropertyKey);
		if (!FileValue.IsEmpty())
		{
			FPlatformMisc::SetEnvironmentVar(Setting.EnvName, *FileValue);
			bAnyFromFile = true;
		}
		else if (Setting.bRequired)
		{
			UE_LOG(LogSentryShaderSymbols, Error, TEXT("Shader symbols upload skipped: set %s in sentry.properties or the %s env var"), Setting.PropertyKey, Setting.EnvName);
			return false;
		}
	}

	if (bAnyFromEnv && bAnyFromFile)
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("Upload settings come from both sentry.properties and environment variables. Make sure that they belong to the same organization and project."));
	}

	const FString ConfigDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir() / TEXT("SentryCli"));
	IFileManager::Get().MakeDirectory(*ConfigDir, true);
	FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_CONFIG_DIR"), *ConfigDir);
	FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_FORCE_ENV_TOKEN"), TEXT("1"));
	FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_CLI_NO_TELEMETRY"), TEXT("1"));
	FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_CLI_NO_UPDATE_CHECK"), TEXT("1"));
	return true;
}

int32 FSentryShaderSymbols::Upload(const FString& FormatDir)
{
	const FString CliPath = FSentrySymToolsDownloader().GetSentryCliPath();
	if (!FPaths::FileExists(CliPath))
	{
		UE_LOG(LogSentryShaderSymbols, Error, TEXT("Shader symbols upload skipped: Sentry CLI not found at %s (download it from the plugin settings)"), *CliPath);
		return 1;
	}

	const FString LogLevel = StaticEnum<ESentryCliLogLevel>()->GetNameStringByValue(static_cast<int64>(GetDefault<USentrySettings>()->DiagnosticLevel)).ToLower();
	const FString Args = FString::Printf(TEXT("debug-files upload --type dxil --log-level %s \"%s\""), *LogLevel, *FPaths::ConvertRelativePathToFull(FormatDir));
	UE_LOG(LogSentryShaderSymbols, Log, TEXT("Uploading shader symbols from %s"), *FormatDir);

	int32 ReturnCode = -1;
	FString StdOut, StdErr;
	if (!FPlatformProcess::ExecProcess(*CliPath, *Args, &ReturnCode, &StdOut, &StdErr))
	{
		UE_LOG(LogSentryShaderSymbols, Error, TEXT("Failed to start %s"), *CliPath);
		return 1;
	}

	TArray<FString> OutLines;
	StdOut.ParseIntoArrayLines(OutLines);
	for (int32 Index = FMath::Max(0, OutLines.Num() - 8); Index < OutLines.Num(); ++Index)
	{
		UE_LOG(LogSentryShaderSymbols, Log, TEXT("  %s"), *OutLines[Index]);
	}
	if (!StdErr.IsEmpty())
	{
		UE_LOG(LogSentryShaderSymbols, Warning, TEXT("%s"), *StdErr);
	}
	UE_LOG(LogSentryShaderSymbols, Log, TEXT("Shader symbols upload finished with code %d"), ReturnCode);
	return ReturnCode;
}
