// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SentrySymbolUploadCommandlet.generated.h"

/**
 * Commandlet for uploading debug symbols to Sentry.
 * Replaces the shell/batch scripts with a native C++ implementation.
 *
 * Usage: UE4Editor-Cmd.exe YourProject -run=SentrySymbolUpload -target-platform=Win64 -target-name=YourGame -target-type=Game -target-configuration=Development -project-dir="C:\Path\To\Project" -plugin-dir="C:\Path\To\Plugin"
 */
UCLASS()
class USentrySymbolUploadCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USentrySymbolUploadCommandlet();

	virtual int32 Main(const FString& Params) override;

private:
	/** Parse command line parameters */
	bool ParseCommandLineParams(const FString& Params);

	/** Check if symbol upload is enabled in plugin settings */
	bool IsSymbolUploadEnabled() const;

	/** Check if upload is enabled for specific target type */
	bool IsTargetTypeEnabled(const FString& TargetType) const;

	/** Check if upload is enabled for specific build configuration */
	bool IsBuildConfigurationEnabled(const FString& TargetConfig) const;

	/** Get the Sentry CLI executable path for current platform */
	FString GetSentryCliPath() const;

	/** Read configuration from DefaultEngine.ini */
	FString ReadConfigValue(const FString& Section, const FString& Key, const FString& DefaultValue = TEXT("")) const;

	/** Check if there is valid sentry.properties file in project root dir */
	bool HasValidSentryPropertiesFile() const;
	/** Check if environment vars required for symbol upload are set */
	bool HasValidSentryEnvVars() const;

	/** Execute the Sentry CLI upload command */
	bool ExecuteSentryCliUpload() const;

private:
	FString TargetPlatform;
	FString TargetType;
	FString TargetConfiguration;
	FString ProjectDir;
	FString PluginDir;
};
