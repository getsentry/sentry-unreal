// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryCallbackUtils.h"

#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

bool SentryCallbackUtils::IsCallbackSafeToRun()
{
	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		return false;
	}

	if (IsGarbageCollecting())
	{
		// If callback is about to be called during garbage collection we can't instantiate UObjects safely or obtain a GC lock
		// since it will cause a deadlock (see https://github.com/getsentry/sentry-unreal/issues/850).
		// In this case a custom callback handler won't be called.
		return false;
	}

	return true;
}