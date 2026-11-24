// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Misc/EngineVersionComparison.h"

/** Native delegates for SDK events that aren't tied to the Sentry subsystem lifetime. */
class SENTRY_API FSentryDelegates
{
public:
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	using FOnAssert = TMulticastDelegate<void(const FString&)>;
#else
	using FOnAssert = TMulticastDelegate<void(const FString&), FDefaultTSDelegateUserPolicy>;
#endif

	/**
	 * Called when an assertion (check, verify, etc.) fails, before the crash is reported.
	 * Only fires while the SDK is initialized, and only for the first failed assertion.
	 * Invoked on the thread that hit the assertion, so handlers must be thread-safe and must not touch UObjects.
	 */
	static FOnAssert OnAssert;
};
