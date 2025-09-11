// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Engine/Engine.h"
#include "SentrySymbolUploadCommandlet.generated.h"

/**
 * Commandlet for uploading debug symbols to Sentry.
 * Replaces the shell/batch scripts with a native C++ implementation.
 * 
 * Usage: [UE4Editor-Cmd.exe | UnrealEditor-Cmd.exe] YourProject -run=SentrySymbolUpload -TargetPlatform=Win64 -TargetName=YourGame -TargetType=Game -TargetConfiguration=Development -ProjectDir="C:\Path\To\Project" -PluginDir="C:\Path\To\Plugin"
 */
UCLASS()
class SENTRYEDITOR_API USentrySymbolUploadCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USentrySymbolUploadCommandlet();

	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface

private:
	/** Parse command line parameters */
	bool ParseCommandLineParams(const FString& Params);
	
	/** Check if symbol upload is enabled in configuration */
	bool IsSymbolUploadEnabled() const;
	
	/** Check if upload is enabled for specific target type */
	bool IsTargetTypeEnabled(const FString& TargetType) const;
	
	/** Check if upload is enabled for specific build configuration */
	bool IsBuildConfigurationEnabled(const FString& TargetConfig) const;
	
	/** Get the Sentry CLI executable path for current platform */
	FString GetSentryCliPath() const;
	
	/** Read configuration from DefaultEngine.ini */
	FString ReadConfigValue(const FString& Section, const FString& Key, const FString& DefaultValue = TEXT("")) const;
	
	/** Read configuration from sentry.properties file */
	bool ReadSentryProperties(FString& ProjectName, FString& OrgName, FString& AuthToken) const;
	
	/** Execute the Sentry CLI upload command */
	bool ExecuteSentryCliUpload() const;
	
	/** Validate all required configuration is present */
	bool ValidateConfiguration() const;

private:
	// Command line parameters
	FString TargetPlatform;
	FString TargetName;
	FString TargetType;
	FString TargetConfiguration;
	FString ProjectDir;
	FString PluginDir;
	
	// Derived paths
	FString ProjectBinariesPath;
	FString PluginBinariesPath;
	FString ConfigPath;
	FString PropertiesFilePath;
};
