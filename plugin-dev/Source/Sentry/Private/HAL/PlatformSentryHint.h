#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryHint.h"
#else
#include "Null/NullSentryHint.h"
#endif

static TSharedPtr<ISentryHint> CreateSharedSentryHint()
{
	return MakeShareable(new FPlatformSentryHint());
}
