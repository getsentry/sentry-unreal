// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySamplingContext.h"
#include "GenericPlatformSentryTransactionContext.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#include "Convenience/GenericPlatformSentryInclude.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentrySamplingContext::FGenericPlatformSentrySamplingContext(sentry_transaction_context_t* transactionContext, sentry_value_t customSamplingContext)
{
	TransactionContext = transactionContext;
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
	const FSentryVariant& samplingContextVariant = FGenericPlatformSentryConverters::VariantToUnreal(CustomSamplingContext);
	if (samplingContextVariant.GetType() == ESentryVariantType::Empty)
	{
		return TMap<FString, FSentryVariant>();
	}

	return samplingContextVariant.GetValue<TMap<FString, FSentryVariant>>();
}

#endif
