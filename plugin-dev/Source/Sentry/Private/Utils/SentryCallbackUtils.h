// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

class SentryCallbackUtils
{
public:
	/**
	 * Checks if it's safe to run callback handlers that instantiate UObjects.
	 * Returns false if during post-load or garbage collection to prevent deadlocks.
	 */
	static bool IsCallbackSafeToRun();
};

/**
 * RAII guard that prevents reentrant invocation of a specific callback type.
 * Uses thread_local storage so each thread tracks reentrancy independently.
 *
 * Template parameter is used solely as a type tag to distinguish callback types -
 * no instances of the tag type are created. Use the handler UClass types as tags
 * (e.g. TSentryCallbackGuard<USentryBeforeSendHandler>).
 */
template<typename CallbackTag>
class TSentryCallbackGuard
{
public:
	TSentryCallbackGuard()
		: bWasAlreadyActive(bIsActive)
	{
		bIsActive = true;
	}

	~TSentryCallbackGuard()
	{
		bIsActive = bWasAlreadyActive;
	}

	/** Returns true if this callback type was already active (reentrant call). */
	bool IsReentrant() const { return bWasAlreadyActive; }

private:
	static thread_local bool bIsActive;
	bool bWasAlreadyActive;
};

template<typename CallbackTag>
thread_local bool TSentryCallbackGuard<CallbackTag>::bIsActive = false;
