// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySamplingContext.h"
#include "GenericPlatformSentryTransactionContext.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#include "Convenience/GenericPlatformSentryInclude.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentrySamplingContext::FGenericPlatformSentrySamplingContext(const sentry_transaction_context_t* transactionContext, sentry_value_t customSamplingContext)
{
	TransactionContext = const_cast<sentry_transaction_context_t*>(transactionContext);
	CustomSamplingContext = customSamplingContext;
}

FGenericPlatformSentrySamplingContext::~FGenericPlatformSentrySamplingContext()
{
	// Put custom destructor logic here if needed
}

TSharedPtr<ISentryTransactionContext> FGenericPlatformSentrySamplingContext::GetTransactionContext() const
{
	return MakeShareable(new FGenericPlatformSentryTransactionContext(TransactionContext));
}

TMap<FString, FSentryVariant> FGenericPlatformSentrySamplingContext::GetCustomSamplingContext() const
{
	return FGenericPlatformSentryConverters::VariantToUnreal(CustomSamplingContext);
}

#endif
