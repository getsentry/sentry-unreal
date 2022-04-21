// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "SentryUserFeedback.h"

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

void USentrySubsystem::Close()
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->Close();
}

void USentrySubsystem::AddBreadcrumb(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->AddBreadcrumb(Message, Category, Type, Data, Level);
}

USentryId* USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureMessage(Message, Level);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureMessageWithScope(Message, OnConfigureScope, Level);
}

USentryId* USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureEvent(Event);
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureEventWithScope(Event, OnConfigureScope);
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	USentryUserFeedback* UserFeedback = NewObject<USentryUserFeedback>();
	UserFeedback->Initialize(EventId);
	UserFeedback->SetEmail(Email);
	UserFeedback->SetComment(Comments);
	UserFeedback->SetName(Name);

	CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::SetUser(USentryUser* User)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetUser(User);
}

void USentrySubsystem::RemoveUser()
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->RemoveUser();
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->ConfigureScope(OnConfigureScope);
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetContext(Key, Values);
}

void USentrySubsystem::SetTag(const FString& Key, const FString& Value)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetTag(Key, Value);
}

void USentrySubsystem::RemoveTag(const FString& Key)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetLevel(Level);
}
