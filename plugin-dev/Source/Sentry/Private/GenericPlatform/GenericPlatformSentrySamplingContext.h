// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentrySamplingContextInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentrySamplingContext : public ISentrySamplingContext
{
public:
	FGenericPlatformSentrySamplingContext(sentry_transaction_context_t* transactionContext, sentry_value_t customSamplingContext);
	virtual ~FGenericPlatformSentrySamplingContext() override;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const override;
	virtual TMap<FString, FSentryVariant> GetCustomSamplingContext() const override;

private:
	sentry_transaction_context_t* TransactionContext;
	sentry_value_t CustomSamplingContext;
};

typedef FGenericPlatformSentrySamplingContext FPlatformSentrySamplingContext;

#endif
