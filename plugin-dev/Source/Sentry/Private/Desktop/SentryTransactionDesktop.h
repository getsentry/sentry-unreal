// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryTransactionInterface.h"

#if USE_SENTRY_NATIVE

class SentryTransactionDesktop : public ISentryTransaction
{
public:
	SentryTransactionDesktop(sentry_transaction_t* transaction);
	virtual ~SentryTransactionDesktop() override;

	sentry_transaction_t* GetNativeObject();

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) override;
	virtual void Finish() override;

private:
	sentry_transaction_t* TransactionDesktop;
};

#endif
