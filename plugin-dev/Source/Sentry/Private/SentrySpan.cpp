// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpan.h"

#include "Interface/SentrySpanInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentrySpanAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySpanApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentrySpanDesktop.h"
#endif

USentrySpan::USentrySpan()
{
}

void USentrySpan::Finish()
{
	if (!SentrySpanNativeImpl)
		return;

	SentrySpanNativeImpl->Finish();
}

void USentrySpan::InitWithNativeImpl(TSharedPtr<ISentrySpan> spanImpl)
{
	SentrySpanNativeImpl = spanImpl;
}

TSharedPtr<ISentrySpan> USentrySpan::GetNativeImpl()
{
	return SentrySpanNativeImpl;
}
