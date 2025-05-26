// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryTransactionContextInterface.h"

#if USE_SENTRY_NATIVE

class ISentryTransaction;

class FGenericPlatformSentryTransactionContext : public ISentryTransactionContext
{
public:
	FGenericPlatformSentryTransactionContext(const FString& name, const FString& operation);
	FGenericPlatformSentryTransactionContext(sentry_transaction_context_t* context);
	virtual ~FGenericPlatformSentryTransactionContext() override = default;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	sentry_transaction_context_t* GetNativeObject();

private:
	sentry_transaction_context_t* TransactionContext;
};

typedef FGenericPlatformSentryTransactionContext FPlatformSentryTransactionContext;

#endif
