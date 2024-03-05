// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

@class SentryTransactionContext;

class SentryTransactionContextApple : public ISentryTransactionContext
{
public:
	SentryTransactionContextApple(const FString& name, const FString& operation);
	SentryTransactionContextApple(SentryTransactionContext* context);
	virtual ~SentryTransactionContextApple() override;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	SentryTransactionContext* GetNativeObject();

private:
	SentryTransactionContext* TransactionContext;
};
