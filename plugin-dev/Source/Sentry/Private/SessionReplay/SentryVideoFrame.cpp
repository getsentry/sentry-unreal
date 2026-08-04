// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoFrame.h"

#ifdef USE_SENTRY_SESSION_REPLAY

bool FSentryVideoFrame::TryAcquire()
{
	if (bInFlight)
	{
		return false;
	}
	bInFlight.AtomicSet(true);
	return true;
}

void FSentryVideoFrame::Release()
{
	bInFlight.AtomicSet(false);
}

#endif // USE_SENTRY_SESSION_REPLAY
