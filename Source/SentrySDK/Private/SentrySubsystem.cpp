// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#if PLATFORM_ANDROID
// TODO
#endif

#if PLATFORM_IOS
#include "IOS/SentryIOS.h"
#endif

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	if(Settings->InitAutomatically)
	{
		Initialize();
	}
}

void USentrySubsystem::Initialize()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

#if PLATFORM_ANDROID
	// TODO
#elif PLATFORM_IOS
	SentryIOS::InitWithSettings(Settings);
#endif
}

FGuid USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
#if PLATFORM_ANDROID
	// TODO
#elif PLATFORM_IOS
	return SentryIOS::CaptureMessage(Message, Level);
#else
	return FGuid();
#endif
}

FGuid USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
#if PLATFORM_ANDROID
	// TODO
#elif PLATFORM_IOS
	return SentryIOS::CaptureMessage(Message, OnConfigureScope, Level);
#else
	return FGuid();
#endif
}

FGuid USentrySubsystem::CaptureError()
{
#if PLATFORM_ANDROID
	// TODO
#elif PLATFORM_IOS
	return SentryIOS::CaptureError();
#else
	return FGuid();
#endif
}
