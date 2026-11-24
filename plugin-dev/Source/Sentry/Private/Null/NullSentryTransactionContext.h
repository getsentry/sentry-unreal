// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

class FNullSentryTransactionContext final : public ISentryTransactionContext
{
public:
	FNullSentryTransactionContext(const FString& name, const FString& operation) {}

	virtual ~FNullSentryTransactionContext() override = default;

	virtual FString GetName() const override { return TEXT(""); }
	virtual FString GetOperation() const override { return TEXT(""); }
};

typedef FNullSentryTransactionContext FPlatformSentryTransactionContext;
