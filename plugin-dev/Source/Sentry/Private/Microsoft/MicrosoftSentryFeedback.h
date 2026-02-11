// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentryFeedback.h"

class FMicrosoftSentryFeedback : public FGenericPlatformSentryFeedback
{
public:
	FMicrosoftSentryFeedback();
	FMicrosoftSentryFeedback(const FString& message);
	virtual ~FMicrosoftSentryFeedback() override = default;

protected:
	virtual void AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment) override;
	virtual void AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment) override;
};

typedef FMicrosoftSentryFeedback FPlatformSentryFeedback;

#endif
