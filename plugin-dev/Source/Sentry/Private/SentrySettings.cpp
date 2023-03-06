// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DsnUrl()
	, InitAutomatically(false)
	, EnableVerboseLogging(true)
	, EnableAutoCrashCapturing(true)
	, EnableAutoLogAttachment(false)
	, UploadSymbolsAutomatically(false)
	, IncludeSources(false)
	, CrashReporterUrl()
{
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), Release, GGameIni);

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
