// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryId.h"

#include "Interface/SentryIdInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryIdAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryIdApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryIdDesktop.h"
#endif

USentryId::USentryId()
{
	if (USentryId::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		SentryIdNativeImpl = MakeShareable(new SentryIdAndroid());
#elif PLATFORM_IOS || PLATFORM_MAC
		SentryIdNativeImpl = MakeShareable(new SentryIdApple());
#elif (PLATFORM_WINDOWS || PLATFORM_LINUX) && USE_SENTRY_NATIVE
		SentryIdNativeImpl = MakeShareable(new SentryIdDesktop());
#endif
	}
}

FString USentryId::ToString() const
{
	if(!SentryIdNativeImpl)
		return FString();

	return SentryIdNativeImpl->ToString();
}

void USentryId::InitWithNativeImpl(TSharedPtr<ISentryId> idImpl)
{
	SentryIdNativeImpl = idImpl;
}

TSharedPtr<ISentryId> USentryId::GetNativeImpl()
{
	return SentryIdNativeImpl;
}
