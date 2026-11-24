// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentrySettings.h"

#include "SessionReplay/SentrySessionReplayRecorder.h"

class ISentrySubsystem;

/**
 * Records a rolling gameplay video that the platform SDK sends along with a crash.
 *
 * Only manages the engine-side recorder lifecycle - picking up and sending recordings is up to the platform SDK.
 */
class FSentrySessionReplayIntegration final : public ISentryIntegration
{
public:
	explicit FSentrySessionReplayIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);

	virtual const TCHAR* GetName() const override { return TEXT("SessionReplay"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	void StartRecording();

	TSharedRef<ISentrySubsystem> NativeImpl;

	FString ReplayDirectory;
	FSentrySessionReplayOptions Options;
	uint32 DurationMs = 0;

	TUniquePtr<FSentrySessionReplayRecorder> Recorder;

	FDelegateHandle EngineLoopInitCompleteHandle;
};

#endif // USE_SENTRY_SESSION_REPLAY
