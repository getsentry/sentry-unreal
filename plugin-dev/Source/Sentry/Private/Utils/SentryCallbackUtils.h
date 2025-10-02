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
