// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentryHint.h"

class FMicrosoftSentryHint : public FGenericPlatformSentryHint
{
public:
	using FGenericPlatformSentryHint::FGenericPlatformSentryHint;

	virtual ~FMicrosoftSentryHint() override = default;

protected:
	virtual void AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment) override;
	virtual void AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment) override;
};

typedef FMicrosoftSentryHint FPlatformSentryHint;

#endif
