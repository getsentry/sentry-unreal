// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "CoreMinimal.h"

@class SentryObjCEvent;

class FAppleSentryReplayEnvelope
{
public:
	/**
	 * Builds a `replay_video` envelope from the recorded clip and its metadata sidecar
	 * and hands it over to sentry-cocoa for sending.
	 *
	 * @param event Crash event from the previous app run used as the source of scope data
	 * (tags, contexts, user, etc.) for the replay; can be nil.
	 * @param videoPath Path to the recorded replay video file.
	 * @param sidecarPath Path to the JSON metadata sidecar describing the clip.
	 *
	 * @return true if the envelope was successfully captured.
	 */
	static bool CaptureForCrashEvent(SentryObjCEvent* event, const FString& videoPath, const FString& sidecarPath);
};

#endif // !USE_SENTRY_NATIVE
