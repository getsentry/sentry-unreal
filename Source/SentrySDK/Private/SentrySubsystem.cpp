// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryEvent.h"

#if PLATFORM_ANDROID
#include "Android/SentrySubsystemAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentrySubsystemIOS.h"
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC
#include "Desktop/SentrySubsystemDesktop.h"
#endif

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_ANDROID
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemAndroid());
#elif PLATFORM_IOS
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemIOS());
#elif PLATFORM_WINDOWS || PLATFORM_MAC
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemDesktop());
#endif

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	if (Settings->InitAutomatically)
	{
		Initialize();
	}
}

void USentrySubsystem::Initialize()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->InitWithSettings(Settings);
}

void USentrySubsystem::AddBreadcrumb(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data,
                                     ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->AddBreadcrumb(Message, Category, Type, Data, Level);
}

FString USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return FString();

	return SubsystemNativeImpl->CaptureMessage(Message, Level);
}

FString USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return FString();

	return SubsystemNativeImpl->CaptureMessage(Message, OnConfigureScope, Level);
}

FString USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	if (!SubsystemNativeImpl)
		return FString();

	return SubsystemNativeImpl->CaptureEvent(Event);
}

FString USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl)
		return FString();

	return SubsystemNativeImpl->CaptureEventWithScope(Event, OnConfigureScope);
}
