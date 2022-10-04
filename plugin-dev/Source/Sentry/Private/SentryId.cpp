// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryId.h"

#include "Interface/SentryIdInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryIdAndroid.h"
#elif PLATFORM_IOS
#include "IOS/SentryIdIOS.h"
#endif

USentryId::USentryId()
{
	if (USentryId::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		SentryIdNativeImpl = MakeShareable(new SentryIdAndroid());
#elif PLATFORM_IOS
		SentryIdNativeImpl = MakeShareable(new SentryIdIOS());
#endif
	}
}

void USentryId::InitWithNativeImpl(TSharedPtr<ISentryId> idImpl)
{
	SentryIdNativeImpl = idImpl;
}

TSharedPtr<ISentryId> USentryId::GetNativeImpl()
{
	return SentryIdNativeImpl;
}
