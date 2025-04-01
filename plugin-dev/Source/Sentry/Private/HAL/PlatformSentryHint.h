#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryHintAndroid.h"
#else
#include "Null/NullSentryHint.h"
#endif

static TSharedPtr<ISentryHint> CreateSharedSentryHint()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryHintAndroid);
#else
	return MakeShareable(new FNullSentryHint);
#endif
}
