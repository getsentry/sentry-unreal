// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

@class SentryId;

class FAppleSentryId : public ISentryId
{
public:
	FAppleSentryId();
	FAppleSentryId(const FString& id);
	FAppleSentryId(SentryId* id);
	virtual ~FAppleSentryId() override;

	SentryId* GetNativeObject();

	virtual FString ToString() const override;

private:
	SentryId* IdApple;
};

typedef FAppleSentryId FPlatformSentryId;