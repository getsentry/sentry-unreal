// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"
#include "SentryDefines.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"

#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InitAutomatically(true)
	, Dsn()
	, Debug(true)
	, Environment(GetDefaultEnvironmentName())
	, SampleRate(1.0f)
	, EnableAutoLogAttachment(false)
	, AttachStacktrace(true)
	, SendDefaultPii(false)
	, AttachScreenshot(false)
	, AttachGpuDump(true)
	, MaxBreadcrumbs(100) 
	, EnableAutoSessionTracking(true)
	, SessionTimeout(30000)
	, OverrideReleaseName(false)
	, UseProxy(false)
	, ProxyUrl()
	, BeforeSendHandler(USentryBeforeSendHandler::StaticClass())
	, EnableAutoCrashCapturing(true)
	, DatabaseLocation(ESentryDatabaseLocation::ProjectUserDirectory)
	, EnableAppNotRespondingTracking(false)
	, EnableTracing(false)
	, SamplingType(ESentryTracesSamplingType::UniformSampleRate)
	, TracesSampleRate(0.0f)
	, TracesSampler(USentryTraceSampler::StaticClass())
	, EnableForPromotedBuildsOnly(false)
	, UploadSymbolsAutomatically(false)	
	, IncludeSources(false)
	, DiagnosticLevel(ESentryCliLogLevel::Info)
	, UseLegacyGradlePlugin(false)
	, CrashReporterUrl()
	, bIsDirty(false)
{
	if (GIsEditor)
	{
		LoadDebugSymbolsProperties();
	}

	CheckLegacySettings();
}

#if WITH_EDITOR

void USentrySettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, InitAutomatically) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, UploadSymbolsAutomatically) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, ProjectName) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, OrgName) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, AuthToken) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, IncludeSources) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, UseLegacyGradlePlugin) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, DiagnosticLevel) ||
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USentrySettings, CrashReporterUrl))
	{
		return;
	}

	bIsDirty = true;
}

#endif

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

bool USentrySettings::IsDirty() const
{
	return bIsDirty;
}

void USentrySettings::ClearDirtyFlag()
{
	bIsDirty = false;
}

FString USentrySettings::GetDefaultEnvironmentName()
{
	if (GIsEditor)
	{
		return TEXT("Editor");
	}

	// Check Shipping configuration separately for backward compatibility
	if(FApp::GetBuildConfiguration() == EBuildConfiguration::Shipping)
	{
		return TEXT("Release");
	}

	return LexToString(FApp::GetBuildConfiguration());
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
		bool UploadSymbols = false;
		GConfig->GetBool(TEXT("/Script/Sentry.SentrySettings"), TEXT("UploadSymbolsAutomatically"), UploadSymbols, *GetDefaultConfigFilename());
		if (UploadSymbols)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Sentry plugin can't find sentry.properties file"));
		}
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
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry settings were marked as dirty (if not checked out in Perforce these need to be updated manually)"));
		GConfig->Flush(false, *ConfigFilename);
	}
}
