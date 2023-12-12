// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

@protocol SentrySpan;

class SentryTransactionApple : public ISentryTransaction
{
public:
	SentryTransactionApple(id<SentrySpan> transaction);
	virtual ~SentryTransactionApple() override;

	id<SentrySpan> GetNativeObject();

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) override;
	virtual void Finish() override;

private:
	id<SentrySpan> TransactionApple;
};
