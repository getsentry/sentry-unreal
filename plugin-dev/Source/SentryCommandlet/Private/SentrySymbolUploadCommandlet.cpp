// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySymbolUploadCommandlet.h"

#include "Interfaces/IPluginManager.h"

#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

#if UE_VERSION_OLDER_THAN(5, 0, 0)
#include "HAL/PlatformFilemanager.h"
#else
#include "HAL/PlatformFileManager.h"
#endif
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"

USentrySymbolUploadCommandlet::USentrySymbolUploadCommandlet()
{
	LogToConsole = true;
	UseCommandletResultAsExitCode = true;
	FastExit = true;

	ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	//PluginDir = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir());
}

int32 USentrySymbolUploadCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("Sentry: Start debug symbols upload"));

	if (!ParseCommandLineParams(Params))
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Failed to parse command line parameters"));
		return 1;
	}

	if (TargetType == TEXT("Editor"))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is not required for Editor target. Skipping..."));
		return 0;
	}

	if (TargetPlatform == TEXT("Android"))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Debug symbols upload for Android is handled by Sentry's Gradle plugin (if enabled)"));
		return 0;
	}

	if (!IsSymbolUploadEnabled())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."));
		return 0;
	}

	if (!IsTargetTypeEnabled(TargetType))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled for target type %s. Skipping..."), *TargetType);
		return 0;
	}

	if (!IsBuildConfigurationEnabled(TargetConfiguration))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled for build configuration %s. Skipping..."), *TargetConfiguration);
		return 0;
	}

	if (!ExecuteSentryCliUpload())
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Symbol upload failed"));
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("Sentry: Upload finished"));
	return 0;
}

bool USentrySymbolUploadCommandlet::ParseCommandLineParams(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("Sentry: Commandlet called with params: %s"), *Params);

	FParse::Value(*Params, TEXT("target-platform="), TargetPlatform);
	FParse::Value(*Params, TEXT("target-type="), TargetType);
	FParse::Value(*Params, TEXT("target-configuration="), TargetConfiguration);

	UE_LOG(LogTemp, Display, TEXT("Sentry: Parsed params - Platform: %s, Type: %s, Config: %s"), *TargetPlatform, *TargetType, *TargetConfiguration);

	if (TargetPlatform.IsEmpty() || TargetType.IsEmpty() || TargetConfiguration.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Missing required command line parameters"));
		return false;
	}

	return true;
}

bool USentrySymbolUploadCommandlet::IsSymbolUploadEnabled() const
{
	FString EnvOverride = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY"));
	if (!EnvOverride.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '%s'"), *EnvOverride);
		return EnvOverride.ToBool();
	}

	FString UploadSymbols = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("UploadSymbolsAutomatically"));
	return UploadSymbols.ToBool();
}

bool USentrySymbolUploadCommandlet::IsTargetTypeEnabled(const FString& InTargetType) const
{
	FString EnabledTargets = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("EnableBuildTargets"));
	if (EnabledTargets.IsEmpty())
	{
		return true; // If not specified, assume enabled
	}

	FString DisabledPattern = FString::Printf(TEXT("bEnable%s=False"), *InTargetType);
	return !EnabledTargets.Contains(DisabledPattern);
}

bool USentrySymbolUploadCommandlet::IsBuildConfigurationEnabled(const FString& InTargetConfig) const
{
	FString EnabledConfigs = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("EnableBuildConfigurations"));
	if (EnabledConfigs.IsEmpty())
	{
		return true; // If not specified, assume enabled
	}

	FString DisabledPattern = FString::Printf(TEXT("bEnable%s=False"), *InTargetConfig);
	return !EnabledConfigs.Contains(DisabledPattern);
}

FString USentrySymbolUploadCommandlet::GetSentryCliPath() const
{
	FString CliFileName;

#if PLATFORM_WINDOWS
	CliFileName = TEXT("sentry-cli-Windows-x86_64.exe");
#elif PLATFORM_MAC
	CliFileName = TEXT("sentry-cli-Darwin-universal");
#elif PLATFORM_LINUX
	CliFileName = TEXT("sentry-cli-Linux-x86_64");
#else
	UE_LOG(LogTemp, Error, TEXT("Sentry: Unsupported platform for Sentry CLI"));
	return FString();
#endif

	FString CliPath = FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("CLI"), CliFileName);

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CliPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Sentry CLI not found at: %s"), *CliPath);
		return FString();
	}

	return CliPath;
}

FString USentrySymbolUploadCommandlet::ReadConfigValue(const FString& Section, const FString& Key, const FString& DefaultValue) const
{
	FString ConfigPath = FPaths::Combine(ProjectDir, TEXT("Config"));
	FString ConfigFilePath = FPaths::Combine(ConfigPath, TEXT("DefaultEngine.ini"));

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ConfigFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Sentry: Config file not found: %s"), *ConfigFilePath);
		return DefaultValue;
	}

	FString Value;
	if (GConfig->GetString(*Section, *Key, Value, ConfigFilePath))
	{
		return Value;
	}

	return DefaultValue;
}

bool USentrySymbolUploadCommandlet::HasValidSentryPropertiesFile() const
{
	FString PropertiesFilePath = FPaths::Combine(ProjectDir, TEXT("sentry.properties"));
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*PropertiesFilePath))
	{
		return false;
	}

	FConfigFile PropertiesFile;
	PropertiesFile.Read(PropertiesFilePath);

	FString ProjectName, OrgName, AuthToken;

	PropertiesFile.GetString(TEXT("Sentry"), TEXT("defaults.project"), ProjectName);
	PropertiesFile.GetString(TEXT("Sentry"), TEXT("defaults.org"), OrgName);
	PropertiesFile.GetString(TEXT("Sentry"), TEXT("auth.token"), AuthToken);

	return !ProjectName.IsEmpty() && !OrgName.IsEmpty() && !AuthToken.IsEmpty();
}

bool USentrySymbolUploadCommandlet::HasValidSentryEnvVars() const
{
	FString SentryProject = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_PROJECT"));
	FString SentryOrg = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_ORG"));
	FString SentryAuthToken = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_AUTH_TOKEN"));

	if (SentryProject.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_PROJECT env var is not set."));
		return false;
	}
	if (SentryOrg.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_ORG env var is not set."));
		return false;
	}
	if (SentryAuthToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_AUTH_TOKEN env var is not set."));
		return false;
	}

	return true;
}

bool USentrySymbolUploadCommandlet::ExecuteSentryCliUpload() const
{
	FString CliPath = GetSentryCliPath();
	if (CliPath.IsEmpty())
	{
		return false;
	}

	if (!HasValidSentryPropertiesFile())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Properties file not found. Falling back to environment variables."));
		if (!HasValidSentryEnvVars())
		{
			return false;
		}
	}
	else
	{
		FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_PROPERTIES"), *FPaths::Combine(ProjectDir, TEXT("sentry.properties")));
	}

	TArray<FString> Arguments;
	Arguments.Add(TEXT("debug-files"));
	Arguments.Add(TEXT("upload"));

	FString IncludeSources = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("IncludeSources"));
	if (IncludeSources.ToBool())
	{
		Arguments.Add(TEXT("--include-sources"));
	}

	FString DiagnosticLogLevel = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("DiagnosticLevel"), TEXT("info"));
	Arguments.Add(TEXT("--log-level"));
	Arguments.Add(DiagnosticLogLevel);

	FString ProjectBinariesPath = FPaths::Combine(ProjectDir, TEXT("Binaries"), TargetPlatform);
	FString PluginBinariesPath = FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TargetPlatform);
	Arguments.Add(ProjectBinariesPath);
	Arguments.Add(PluginBinariesPath);

	int32 ReturnCode;
	FString StdOut, StdErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*CliPath, *FString::Join(Arguments, TEXT(" ")), &ReturnCode, &StdOut, &StdErr);

	if (!bSuccess || ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry CLI failed with return code %d"), ReturnCode);
		if (!StdErr.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Error output: %s"), *StdErr);
		}
		return false;
	}

	if (!StdOut.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: CLI output: %s"), *StdOut);
	}

	return true;
}
