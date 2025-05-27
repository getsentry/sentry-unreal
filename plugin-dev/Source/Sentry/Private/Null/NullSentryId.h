// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

class FNullSentryId final : public ISentryId
{
public:
	FNullSentryId(const FString& Id) {}

	virtual ~FNullSentryId() override = default;

	virtual FString ToString() const override { return TEXT(""); }
};

typedef FNullSentryId FPlatformSentryId;
