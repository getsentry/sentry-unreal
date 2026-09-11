// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentryHintInterface.h"

@class SentryObjCHint;

class FAppleSentryHint : public ISentryHint
{
public:
	FAppleSentryHint();
	FAppleSentryHint(SentryObjCHint* hint);
	virtual ~FAppleSentryHint() override;

	SentryObjCHint* GetNativeObject();

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;

private:
	SentryObjCHint* HintApple;
};

typedef FAppleSentryHint FPlatformSentryHint;

#endif // !USE_SENTRY_NATIVE
