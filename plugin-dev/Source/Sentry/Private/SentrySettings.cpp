// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySettings.h"
#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentryTraceSampler.h"

#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

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
	, MaxAttachmentSize(20 * 1024 * 1024)
	, MaxBreadcrumbs(100)
	, EnableAutoSessionTracking(true)
	, SessionTimeout(30000)
	, OverrideReleaseName(false)
	, UseProxy(false)
	, ProxyUrl()
	, BeforeSendHandler(nullptr)
	, BeforeBreadcrumbHandler(nullptr)
	, EnableAutoCrashCapturing(true)
	, DatabaseLocation(ESentryDatabaseLocation::ProjectUserDirectory)
	, CrashpadWaitForUpload(false)
	, EnableAppNotRespondingTracking(false)
	, EnableTracing(false)
	, SamplingType(ESentryTracesSamplingType::UniformSampleRate)
	, TracesSampleRate(0.0f)
	, TracesSampler(nullptr)
	, EditorDsn()
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

FString USentrySettings::GetEffectiveDsn() const
{
	return GIsEditor && !EditorDsn.IsEmpty() ? EditorDsn : Dsn;
}

FString USentrySettings::GetFormattedReleaseName()
{
	FString FormattedReleaseName = FApp::GetProjectName();

	FString Version = TEXT("");
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), Version, GGameIni);
	if (!Version.IsEmpty())
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
	if (FApp::GetBuildConfiguration() == EBuildConfiguration::Shipping)
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
