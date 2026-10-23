// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryStateReplaySettings.h"

USentryStateReplaySettings::USentryStateReplaySettings()
	: bEnabled(false)
	, bAutoStart(true)
	, SampleRateHz(10)
	, bCaptureUI(true)
	, OutputSubdir(TEXT("StateReplays"))
	, FlushIntervalSeconds(1.0f)
{
}
