// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#ifdef SENTRY_REPLAY_ENCODER_AVCODECS
#include "SentryVideoEncoder.h"
#else
#include COMPILED_PLATFORM_HEADER(SentryVideoEncoder.h)
#endif

#endif // USE_SENTRY_SESSION_REPLAY
