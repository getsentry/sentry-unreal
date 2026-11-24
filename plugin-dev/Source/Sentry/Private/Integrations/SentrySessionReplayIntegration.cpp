// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentrySessionReplayIntegration.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentrySettings.h"

#include "Interface/SentrySubsystemInterface.h"

#include "CoreGlobals.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

FSentrySessionReplayIntegration::FSentrySessionReplayIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

bool FSentrySessionReplayIntegration::Setup(const USentrySettings& Settings)
{
	if (!Settings.AttachSessionReplay)
	{
		return false;
	}

	ReplayDirectory = NativeImpl->GetSessionReplayDirectory();
	if (ReplayDirectory.IsEmpty())
	{
		return false;
	}

	Options = Settings.SessionReplayOptions;
	DurationMs = Settings.SessionReplayDurationMs;

	// The recorder hooks the Slate backbuffer, which isn't available yet if Sentry is initialized
	// before the engine loop finishes. Defer the start until engine init completes in that case.
	if (GIsRunning)
	{
		StartRecording();
	}
	else
	{
		// OnFEngineLoopInitComplete broadcasts exactly once. The binding is removed in Teardown() rather than
		// from inside the callback, which would unbind the delegate instance while it's being executed.
		EngineLoopInitCompleteHandle = FCoreDelegates::OnFEngineLoopInitComplete.AddRaw(this, &FSentrySessionReplayIntegration::StartRecording);
	}

	return true;
}

void FSentrySessionReplayIntegration::Teardown()
{
	if (EngineLoopInitCompleteHandle.IsValid())
	{
		FCoreDelegates::OnFEngineLoopInitComplete.Remove(EngineLoopInitCompleteHandle);
		EngineLoopInitCompleteHandle.Reset();
	}

	if (Recorder)
	{
		// Stops the capture and joins the recorder threads
		Recorder->Shutdown();
		Recorder.Reset();
	}
}

void FSentrySessionReplayIntegration::StartRecording()
{
	const FString ReplayId = FGuid::NewGuid().ToString(EGuidFormats::Digits).ToLower();
	const FString ReplayPath = FPaths::Combine(ReplayDirectory, FString::Printf(TEXT("replay-%s.mp4"), *ReplayId));

	Recorder = MakeUnique<FSentrySessionReplayRecorder>();
	if (!Recorder->Initialize(Options, DurationMs, ReplayId, ReplayPath))
	{
		Recorder.Reset();
		return;
	}

	NativeImpl->SetContext(TEXT("replay"), { { TEXT("replay_id"), FSentryVariant(ReplayId) } });
	NativeImpl->SetAttribute(TEXT("sentry.replay_id"), FSentryVariant(ReplayId));
	NativeImpl->SetAttribute(TEXT("sentry._internal.replay_is_buffering"), FSentryVariant(true));
}

#endif // USE_SENTRY_SESSION_REPLAY
