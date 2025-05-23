// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Null/NullSentrySpan.h"

static TSharedPtr<ISentrySpan> CreateSharedSentrySpan()
{
	// Span is supposed to be created internally by the SDK using the platform-specific implementations.
	// Currently, it doesn't provide default constructor for Apple/Android thus we can only return Null-version here.
	return MakeShareable(new FPlatformSentrySpan);
}
