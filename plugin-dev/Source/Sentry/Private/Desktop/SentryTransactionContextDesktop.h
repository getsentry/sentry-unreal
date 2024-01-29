// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryTransactionContextInterface.h"

#if USE_SENTRY_NATIVE

class SentryTransactionContextDesktop : public ISentryTransactionContext
{
public:
	SentryTransactionContextDesktop(const FString& name, const FString& operation);
	SentryTransactionContextDesktop(sentry_transaction_context_t* context);
	virtual ~SentryTransactionContextDesktop() override;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	sentry_transaction_context_t* GetNativeObject();

private:
	sentry_transaction_context_t* TransactionContextDesktop;
};

#endif
