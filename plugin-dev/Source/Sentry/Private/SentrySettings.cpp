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
	, Environment()
	, Dist()
	, SampleRate(1.0f)
	, EnableAutoLogAttachment(false)
	, AttachStacktrace(true)
	, SendDefaultPii(false)
	, AttachScreenshot(false)
	, AttachGpuDump(true)
	, MaxAttachmentSize(20 * 1024 * 1024)
	, EnableStructuredLogging(false)
	, StructuredLoggingCategories()
	, StructuredLoggingLevels()
	, bSendBreadcrumbsWithStructuredLogging(false)
	, MaxBreadcrumbs(100)
	, AutomaticBreadcrumbs()
	, AutomaticBreadcrumbsForLogs()
	, EnableAutoSessionTracking(true)
	, SessionTimeout(30000)
	, OverrideReleaseName(false)
	, Release()
	, UseProxy(false)
	, ProxyUrl()
	, BeforeSendHandler(nullptr)
	, BeforeBreadcrumbHandler(nullptr)
	, BeforeLogHandler(nullptr)
	, EnableAutoCrashCapturing(true)
	, DatabaseLocation(ESentryDatabaseLocation::ProjectUserDirectory)
	, CrashpadWaitForUpload(false)
	, InAppInclude()
	, InAppExclude()
	, EnableAppNotRespondingTracking(false)
	, EnableTracing(false)
	, SamplingType(ESentryTracesSamplingType::UniformSampleRate)
	, TracesSampleRate(0.0f)
	, TracesSampler(nullptr)
	, EditorDsn()
	, TagsPromotion()
	, EnableBuildConfigurations()
	, EnableBuildTargets()
	, EnableForPromotedBuildsOnly(false)
	, UploadSymbolsAutomatically(false)
	, ProjectName()
	, OrgName()
	, AuthToken()
	, IncludeSources(false)
	, DiagnosticLevel(ESentryCliLogLevel::Info)
	, UseLegacyGradlePlugin(false)
	, CrashReporterUrl()
	, bRequireUserConsent(false)
	, bDefaultUserConsentGiven(true)
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
	if (GIsEditor && !EditorDsn.IsEmpty())
	{
		return EditorDsn;
	}

	if (!Dsn.IsEmpty())
	{
		return Dsn;
	}

	const FString& EnvVarDsn = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_DSN"));
	if (!EnvVarDsn.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("DSN is not set in plugin settings - using SENTRY_DSN environment variable instead."));
		return EnvVarDsn;
	}

	UE_LOG(LogSentrySdk, Log, TEXT("DSN is not configured."));
	return FString();
}

FString USentrySettings::GetEffectiveEnvironment() const
{
	if (!Environment.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Using the value from plugin settings as Sentry environment."));
		return Environment;
	}

	const FString& EnvVarEnvironment = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_ENVIRONMENT"));
	if (!EnvVarEnvironment.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Using SENTRY_ENVIRONMENT variable as Sentry environment."));
		return EnvVarEnvironment;
	}

	UE_LOG(LogSentrySdk, Log, TEXT("Using current build configuration as Sentry environment."));
	return GetEnvironmentFromBuildConfig();
}

FString USentrySettings::GetEnvironmentFromBuildConfig() const
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

FString USentrySettings::GetEffectiveRelease() const
{
	if (OverrideReleaseName)
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Using the value from plugin settings as Sentry release."));
		return Release;
	}

	const FString& EnvVarRelease = FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_RELEASE"));
	if (!EnvVarRelease.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Using SENTRY_RELEASE variable as Sentry release."));
		return EnvVarRelease;
	}

	UE_LOG(LogSentrySdk, Log, TEXT("Using current project name and version as Sentry release."));
	return GetReleaseFromProjectSettings();
}

FString USentrySettings::GetReleaseFromProjectSettings() const
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
