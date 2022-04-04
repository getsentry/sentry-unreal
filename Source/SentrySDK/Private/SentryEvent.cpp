// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "Interface/SentryEventInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryEventAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentryEventIOS.h"
#endif

USentryEvent::USentryEvent()
{
	if (USentryEvent::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		EventNativeImpl = MakeShareable(new SentryEventAndroid());
#endif
#if PLATFORM_IOS
		EventNativeImpl = MakeShareable(new SentryEventIOS());
#endif
	}
}

void USentryEvent::SetMessage(const FString& Message)
{
	if (!EventNativeImpl)
		return;

	EventNativeImpl->SetMessage(Message);
}

void USentryEvent::SetLevel(ESentryLevel Level)
{
	if (!EventNativeImpl)
		return;

	EventNativeImpl->SetLevel(Level);
}

void USentryEvent::InitWithNativeImpl(TSharedPtr<ISentryEvent> eventImpl)
{
	if (!EventNativeImpl)
		return;

	EventNativeImpl = eventImpl;
}

TSharedPtr<ISentryEvent> USentryEvent::GetNativeImpl()
{
	return EventNativeImpl;
}
