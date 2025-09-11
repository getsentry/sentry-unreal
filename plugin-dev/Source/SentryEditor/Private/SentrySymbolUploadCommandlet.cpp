// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySymbolUploadCommandlet.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

USentrySymbolUploadCommandlet::USentrySymbolUploadCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	
	UE_LOG(LogTemp, Display, TEXT("Sentry: USentrySymbolUploadCommandlet constructor called"));
}

int32 USentrySymbolUploadCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Log, TEXT("Sentry: Start debug symbols upload"));

	// Parse command line parameters
	if (!ParseCommandLineParams(Params))
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Failed to parse command line parameters"));
		return 1;
	}

	// Skip editor builds
	if (TargetType == TEXT("Editor"))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is not required for Editor target. Skipping..."));
		return 0;
	}

	// Skip Android builds (handled by Gradle plugin)
	if (TargetPlatform == TEXT("Android"))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Debug symbols upload for Android is handled by Sentry's Gradle plugin (if enabled)"));
		return 0;
	}

	// Check if symbol upload is enabled
	if (!IsSymbolUploadEnabled())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."));
		return 0;
	}

	// Check target type configuration
	if (!IsTargetTypeEnabled(TargetType))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled for target type %s. Skipping..."), *TargetType);
		return 0;
	}

	// Check build configuration
	if (!IsBuildConfigurationEnabled(TargetConfiguration))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload is disabled for build configuration %s. Skipping..."), *TargetConfiguration);
		return 0;
	}

	// Validate configuration
	if (!ValidateConfiguration())
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Configuration validation failed"));
		return 1;
	}

	// Execute upload
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
	
	// Parse parameters from command line
	FParse::Value(*Params, TEXT("SentryTargetPlatform="), TargetPlatform);
	FParse::Value(*Params, TEXT("SentryTargetName="), TargetName);
	FParse::Value(*Params, TEXT("SentryTargetType="), TargetType);
	FParse::Value(*Params, TEXT("SentryTargetConfiguration="), TargetConfiguration);
	FParse::Value(*Params, TEXT("SentryProjectDir="), ProjectDir);
	FParse::Value(*Params, TEXT("SentryPluginDir="), PluginDir);

	// Remove quotes if present
	ProjectDir = ProjectDir.TrimQuotes();
	PluginDir = PluginDir.TrimQuotes();

	UE_LOG(LogTemp, Display, TEXT("Sentry: Parsed params - Platform: %s, Name: %s, Type: %s, Config: %s"), *TargetPlatform, *TargetName, *TargetType, *TargetConfiguration);
	
	// Validate required parameters
	if (TargetPlatform.IsEmpty() || TargetName.IsEmpty() || TargetType.IsEmpty() || 
		TargetConfiguration.IsEmpty() || ProjectDir.IsEmpty() || PluginDir.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Missing required command line parameters"));
		return false;
	}

	// Set up derived paths
	ProjectBinariesPath = FPaths::Combine(ProjectDir, TEXT("Binaries"), TargetPlatform);
	PluginBinariesPath = FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TargetPlatform);
	ConfigPath = FPaths::Combine(ProjectDir, TEXT("Config"));
	PropertiesFilePath = FPaths::Combine(ProjectDir, TEXT("sentry.properties"));

	return true;
}

bool USentrySymbolUploadCommandlet::IsSymbolUploadEnabled() const
{
	// Check environment variable override first
	FString EnvOverride = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY"));
	if (!EnvOverride.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '%s'"), *EnvOverride);
		return EnvOverride.ToBool();
	}

	// Check configuration file
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

	// Remove parentheses
	EnabledTargets = EnabledTargets.TrimStartAndEnd().Mid(1, EnabledTargets.Len() - 2);
	
	// Check if this target type is disabled
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

	// Remove parentheses
	EnabledConfigs = EnabledConfigs.TrimStartAndEnd().Mid(1, EnabledConfigs.Len() - 2);
	
	// Check if this build configuration is disabled
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
	UE_LOG(LogTemp, Error, TEXT("Sentry: Unsupported platform for CLI"));
	return FString();
#endif

	FString CliPath = FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("CLI"), CliFileName);
	
	// Verify the CLI exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CliPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Sentry CLI not found at: %s"), *CliPath);
		return FString();
	}

	return CliPath;
}

FString USentrySymbolUploadCommandlet::ReadConfigValue(const FString& Section, const FString& Key, const FString& DefaultValue) const
{
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

bool USentrySymbolUploadCommandlet::ReadSentryProperties(FString& ProjectName, FString& OrgName, FString& AuthToken) const
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*PropertiesFilePath))
	{
		return false;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *PropertiesFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Sentry: Failed to read properties file: %s"), *PropertiesFilePath);
		return false;
	}

	for (const FString& Line : Lines)
	{
		if (Line.StartsWith(TEXT("defaults.project=")))
		{
			ProjectName = Line.Mid(17); // Length of "defaults.project="
		}
		else if (Line.StartsWith(TEXT("defaults.org=")))
		{
			OrgName = Line.Mid(13); // Length of "defaults.org="
		}
		else if (Line.StartsWith(TEXT("auth.token=")))
		{
			AuthToken = Line.Mid(11); // Length of "auth.token="
		}
	}

	return !ProjectName.IsEmpty() && !OrgName.IsEmpty() && !AuthToken.IsEmpty();
}

bool USentrySymbolUploadCommandlet::ValidateConfiguration() const
{
	// Check if CLI exists
	FString CliPath = GetSentryCliPath();
	if (CliPath.IsEmpty())
	{
		return false;
	}

	// Try to read from properties file first
	FString ProjectName, OrgName, AuthToken;
	if (ReadSentryProperties(ProjectName, OrgName, AuthToken))
	{
		UE_LOG(LogTemp, Display, TEXT("Sentry: Properties file found. Configuration validated."));
		return true;
	}

	// Fall back to environment variables
	UE_LOG(LogTemp, Display, TEXT("Sentry: Properties file not found. Falling back to environment variables."));
	
	FString SentryProject = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_PROJECT"));
	FString SentryOrg = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_ORG"));
	FString SentryAuthToken = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_AUTH_TOKEN"));

	if (SentryProject.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_PROJECT env var is not set. Skipping..."));
		return false;
	}
	if (SentryOrg.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_ORG env var is not set. Skipping..."));
		return false;
	}
	if (SentryAuthToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Error: SENTRY_AUTH_TOKEN env var is not set. Skipping..."));
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

	// Build command line arguments
	TArray<FString> Arguments;
	Arguments.Add(TEXT("debug-files"));
	Arguments.Add(TEXT("upload"));

	// Check if sources should be included
	FString IncludeSources = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("IncludeSources"));
	if (IncludeSources.ToBool())
	{
		Arguments.Add(TEXT("--include-sources"));
	}

	// Add log level
	FString DiagnosticLogLevel = ReadConfigValue(TEXT("/Script/Sentry.SentrySettings"), TEXT("DiagnosticLevel"), TEXT("info"));
	Arguments.Add(TEXT("--log-level"));
	Arguments.Add(DiagnosticLogLevel);

	// Add paths to upload
	Arguments.Add(ProjectBinariesPath);
	Arguments.Add(PluginBinariesPath);

	// Set environment variables if using properties file
	FString ProjectName, OrgName, AuthToken;
	if (ReadSentryProperties(ProjectName, OrgName, AuthToken))
	{
		FPlatformMisc::SetEnvironmentVar(TEXT("SENTRY_PROPERTIES"), *PropertiesFilePath);
	}

	// Execute the CLI
	UE_LOG(LogTemp, Display, TEXT("Sentry: Executing CLI: %s"), *CliPath);
	
	int32 ReturnCode;
	FString StdOut, StdErr;
	
	bool bSuccess = FPlatformProcess::ExecProcess(
		*CliPath,
		*FString::Join(Arguments, TEXT(" ")),
		&ReturnCode,
		&StdOut,
		&StdErr
	);

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
		UE_LOG(LogTemp, Display, TEXT("CLI output: %s"), *StdOut);
	}

	return true;
}
