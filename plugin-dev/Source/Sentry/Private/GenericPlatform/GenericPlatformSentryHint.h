// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryHintInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryAttachment;

class FGenericPlatformSentryHint : public ISentryHint
{
public:
	FGenericPlatformSentryHint();
	FGenericPlatformSentryHint(sentry_hint_t* hint);
	virtual ~FGenericPlatformSentryHint() override = default;

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;

protected:
	virtual void AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment);
	virtual void AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment);

	sentry_hint_t* Hint;
};

#if !PLATFORM_MICROSOFT
typedef FGenericPlatformSentryHint FPlatformSentryHint;
#endif

#endif
