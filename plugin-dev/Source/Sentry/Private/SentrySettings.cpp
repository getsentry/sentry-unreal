// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

#include "SentryBeforeSendHandler.h"

#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DsnUrl()
	, InitAutomatically(true)
	, EnableVerboseLogging(true)
	, EnableAutoCrashCapturing(true)
	, EnableAutoLogAttachment(false)
	, EnableStackTrace(true)
	, UseProxy(false) 
	, ProxyUrl()
	, SampleRate(1.0f)
	, MaxBreadcrumbs(100)
	, AttachScreenshots(false)
	, SendDafaultPii(false)
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
}

FString USentrySettings::GetFormattedReleaseName() const
{
	FString FormattedReleaseName = FApp::GetProjectName();

	FString Version;
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
}
