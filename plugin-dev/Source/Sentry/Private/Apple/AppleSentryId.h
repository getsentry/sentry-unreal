// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentryIdInterface.h"

@class SentryObjCId;

class FAppleSentryId : public ISentryId
{
public:
	FAppleSentryId();
	FAppleSentryId(const FString& id);
	FAppleSentryId(SentryObjCId* id);
	virtual ~FAppleSentryId() override;

	SentryObjCId* GetNativeObject();

	virtual FString ToString() const override;

private:
	SentryObjCId* IdApple;
};

typedef FAppleSentryId FPlatformSentryId;

#endif // !USE_SENTRY_NATIVE