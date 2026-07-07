// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

/**
 * Verifies that a session replay staged in the Sentry database directory is picked up
 * by the native backend's crash daemon, wrapped into a `replay_video` envelope and sent
 * to Sentry alongside the crash event.
 *
 * Since CI runners can't render (tests run with -nullrhi), the actual recorder never
 * starts. Instead, the test emulates its on-disk output: it stages a replay clip with
 * a metadata sidecar in `<database>/replays/` and sets the `replay` context on the
 * scope - exactly what the recorder does when active. The daemon doesn't parse the
 * video content, and Sentry stores it as an opaque blob, so the clip is filler bytes
 * generated at runtime rather than a real recording.
 *
 * Only meaningful with the native crash backend (`UseNativeBackend=True`) - other
 * backends don't run the daemon that flushes staged replays.
 */
class FSentryReplayTest : public FSentryBaseIntegrationTest
{
public:
	FSentryReplayTest() : FSentryBaseIntegrationTest(TEXT("replay-capture")) {}

	virtual void Run() override;
};
