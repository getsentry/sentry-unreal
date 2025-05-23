// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

@class SentryTransactionContext;

class FAppleSentryTransactionContext : public ISentryTransactionContext
{
public:
	FAppleSentryTransactionContext(const FString& name, const FString& operation);
	FAppleSentryTransactionContext(SentryTransactionContext* context);
	virtual ~FAppleSentryTransactionContext() override;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	SentryTransactionContext* GetNativeObject();

private:
	SentryTransactionContext* TransactionContext;
};

typedef FAppleSentryTransactionContext FPlatformSentryTransactionContext;
