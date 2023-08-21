// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"
#include "SentryDefines.h"
#include "SentryBeforeSendHandler.h"

#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Dsn()
	, InitAutomatically(true)
	, Debug(true)
	, EnableForPromotedBuildsOnly(true)
	, EnableAutoCrashCapturing(true)
	, EnableAutoLogAttachment(false)
	, AttachStacktrace(true)
	, UseProxy(false) 
	, ProxyUrl()
	, SampleRate(1.0f)
	, MaxBreadcrumbs(100)
	, AttachScreenshot(false)
	, SendDefaultPii(false)
	, EnableAutoSessionTracking(true)
	, SessionTimeout(30000)
	, OverrideReleaseName(false)
	, UploadSymbolsAutomatically(false)
	, IncludeSources(false)
	, CrashReporterUrl()
	, BeforeSendHandler(USentryBeforeSendHandler::StaticClass())
{
#if WITH_EDITOR
	Environment = TEXT("Editor");
#elif UE_BUILD_SHIPPING
	Environment = TEXT("Release");
#elif UE_BUILD_DEVELOPMENT
	Environment = TEXT("Development");
#elif UE_BUILD_DEBUG
	Environment = TEXT("Debug");
#endif

	LoadDebugSymbolsProperties();

	CheckLegacySettings();
}

FString USentrySettings::GetFormattedReleaseName()
{
	FString FormattedReleaseName = FApp::GetProjectName();

	FString Version = TEXT("");
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), Version, GGameIni);
	if(!Version.IsEmpty())
	{
		FormattedReleaseName = FString::Printf(TEXT("%s@%s"), *FormattedReleaseName, *Version);
	}

	return FormattedReleaseName;
}

void USentrySettings::LoadDebugSymbolsProperties()
{
	const FString PropertiesFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("sentry.properties"));

	if (FPaths::FileExists(PropertiesFilePath))
	{
		FConfigFile PropertiesFile;
		PropertiesFile.Read(PropertiesFilePath);

		PropertiesFile.GetString(TEXT("Sentry"), TEXT("defaults.project"), ProjectName);
		PropertiesFile.GetString(TEXT("Sentry"), TEXT("defaults.org"), OrgName);
		PropertiesFile.GetString(TEXT("Sentry"), TEXT("auth.token"), AuthToken);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Sentry plugin can't find properties file"));
	}
}

void USentrySettings::CheckLegacySettings()
{
	bool IsSettingsDirty = false;

	const FString SentrySection = TEXT("/Script/Sentry.SentrySettings");
	const FString ConfigFilename = GetDefaultConfigFilename();

	// Settings renamed in 0.9.0

	const FString DsnLegacyKey = TEXT("DsnUrl");
	FString DsnLegacyValue = TEXT("");
	if(GConfig->GetString(*SentrySection, *DsnLegacyKey, DsnLegacyValue, *ConfigFilename))
	{
		Dsn = DsnLegacyValue;
		GConfig->SetString(*SentrySection, TEXT("Dsn"), *Dsn, *ConfigFilename);
		GConfig->RemoveKey(*SentrySection, *DsnLegacyKey, *ConfigFilename);
		IsSettingsDirty = true;
	}

	const FString DebugLegacyKey = TEXT("EnableVerboseLogging");
	bool DebugLegacyValue;
	if(GConfig->GetBool(*SentrySection, *DebugLegacyKey, DebugLegacyValue, *ConfigFilename))
	{
		Debug = DebugLegacyValue;
		GConfig->SetBool(*SentrySection, TEXT("Debug"), Debug, *ConfigFilename);
		GConfig->RemoveKey(*SentrySection, *DebugLegacyKey, *ConfigFilename);
		IsSettingsDirty = true;
	}

	const FString AttachStacktraceLegacyKey = TEXT("EnableStackTrace");
	bool AttachStacktraceLegacyValue;
	if(GConfig->GetBool(*SentrySection, *AttachStacktraceLegacyKey, AttachStacktraceLegacyValue, *ConfigFilename))
	{
		AttachStacktrace = AttachStacktraceLegacyValue;
		GConfig->SetBool(*SentrySection, TEXT("AttachStacktrace"), AttachStacktrace, *ConfigFilename);
		GConfig->RemoveKey(*SentrySection, *AttachStacktraceLegacyKey, *ConfigFilename);
		IsSettingsDirty = true;
	}

	// Place newly renamed settings here specifying the release for which changes take place

	if (IsSettingsDirty)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry settings where marked as dirty - If not checked out in Perforce, you'll need to update these manually"));
		GConfig->Flush(false, *ConfigFilename);
	}
}