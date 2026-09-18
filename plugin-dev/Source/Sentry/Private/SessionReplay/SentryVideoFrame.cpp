// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoFrame.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "RHIGPUReadback.h"
#include "RHIResources.h"

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

bool FSentryVideoFrame::IsGpuWriteComplete() const
{
	// Software encoders read back through FRHIGPUTextureReadback, which tracks
	// its own completion; the fence only guards the hardware path
	if (Readback.IsValid())
	{
		return Readback->IsReady();
	}
	return !ReadyFence.IsValid() || (ReadyFence->NumPendingWriteCommands.GetValue() == 0 && ReadyFence->Poll());
}

#endif // USE_SENTRY_SESSION_REPLAY
