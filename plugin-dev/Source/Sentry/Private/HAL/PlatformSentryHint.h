#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryHint.h"
#else
#include "Null/NullSentryHint.h"
#endif

static TSharedPtr<ISentryHint> CreateSharedSentryHint()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryHint);
#else
	return MakeShareable(new FNullSentryHint);
#endif
}
