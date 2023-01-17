// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

@class SentryId;

class SentryIdApple : public ISentryId
{
public:
	SentryIdApple();
	SentryIdApple(SentryId* id);
	virtual ~SentryIdApple() override;

	SentryId* GetNativeObject();

	virtual FString ToString() const override;

private:
	SentryId* IdApple;
};