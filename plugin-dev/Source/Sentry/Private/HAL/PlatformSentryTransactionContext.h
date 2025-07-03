// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryTransactionContext.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryTransactionContext.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransactionContext.h"
#else
#include "Null/NullSentryTransactionContext.h"
#endif

static TSharedPtr<ISentryTransactionContext> CreateSharedSentryTransactionContext(const FString& Name, const FString& Operation)
{
	return MakeShareable(new FPlatformSentryTransactionContext(Name, Operation));
}
