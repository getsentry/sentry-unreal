// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"

#if PLATFORM_ANDROID
#include "Android/SentryEventAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentryEventIOS.h"
#endif

USentryEvent::USentryEvent()
{
#if PLATFORM_ANDROID
	if (USentryEvent::StaticClass()->GetDefaultObject() != this)
	{
		_eventNativeImplAndroid = MakeShareable(new SentryEventAndroid());
	}
#endif
#if PLATFORM_IOS
	if (USentryEvent::StaticClass()->GetDefaultObject() != this)
	{
		_eventNativeImplIOS = MakeShareable(new SentryEventIOS());
	}
#endif
}

void USentryEvent::SetMessage(const FString& Message)
{
#if PLATFORM_ANDROID
	_eventNativeImplAndroid->SetMessage(Message);
#endif
#if PLATFORM_IOS
	_eventNativeImplIOS->SetMessage(Message);
#endif
}

void USentryEvent::SetLevel(ESentryLevel Level)
{
#if PLATFORM_ANDROID
	_eventNativeImplAndroid->SetLevel(Level);
#endif
#if PLATFORM_IOS
	_eventNativeImplIOS->SetLevel(Level);
#endif
}

#if PLATFORM_ANDROID
void USentryEvent::InitWithNativeImplAndroid(TSharedPtr<SentryEventAndroid> eventImpl)
{
	_eventNativeImplAndroid = eventImpl;
}

TSharedPtr<SentryEventAndroid> USentryEvent::GetNativeImplAndroid()
{
	return _eventNativeImplAndroid;
}
#endif

#if PLATFORM_IOS
void USentryEvent::InitWithNativeImplIOS(TSharedPtr<SentryEventIOS> eventImpl)
{
	_eventNativeImplIOS = eventImpl;
}

TSharedPtr<SentryEventIOS> USentryEvent::GetNativeImplIOS()
{
	return _eventNativeImplIOS;
}
#endif
