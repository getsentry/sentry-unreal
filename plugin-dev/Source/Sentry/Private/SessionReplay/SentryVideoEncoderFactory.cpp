// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "ISentryVideoEncoder.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#ifdef SENTRY_REPLAY_ENCODER_OPENH264
#include "SentryOpenH264Encoder.h"
#endif

#ifdef SENTRY_REPLAY_ENCODER_AVCODECS
#include "SentryVideoEncoder.h"
#endif

TUniquePtr<ISentryVideoEncoder> CreateSentryVideoEncoder(FSentrySessionReplayRecorder& Recorder,
	uint32 Framerate, int32 BitrateKbps, float FragmentSeconds, int32 MaxCaptureHeight)
{
	// A platform builds with exactly one backend: openh264 where the platform
	// offers no encoder, AVCodecs everywhere else. Build.cs decides which.
#if defined(SENTRY_REPLAY_ENCODER_OPENH264)
	return MakeUnique<FSentryOpenH264Encoder>(Recorder, Framerate, BitrateKbps, FragmentSeconds, MaxCaptureHeight);
#elif defined(SENTRY_REPLAY_ENCODER_AVCODECS)
	// The hardware path takes the backbuffer as a GPU resource and has no cheap
	// place to rescale, so MaxCaptureHeight only applies to the software encoder
	return MakeUnique<FSentryVideoEncoder>(Recorder, Framerate, BitrateKbps, FragmentSeconds);
#else
	return nullptr;
#endif
}

#endif // USE_SENTRY_SESSION_REPLAY
