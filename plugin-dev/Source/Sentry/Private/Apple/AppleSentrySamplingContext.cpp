// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySamplingContext.h"

#if !USE_SENTRY_NATIVE

#include "AppleSentryTransactionContext.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

FAppleSentrySamplingContext::FAppleSentrySamplingContext(SentrySamplingContext* context)
{
	SamplingContext = context;
}

FAppleSentrySamplingContext::~FAppleSentrySamplingContext()
{
	// Put custom destructor logic here if needed
}

TSharedPtr<ISentryTransactionContext> FAppleSentrySamplingContext::GetTransactionContext() const
{
	return MakeShareable(new FAppleSentryTransactionContext(SamplingContext.transactionContext));
}

TMap<FString, FSentryVariant> FAppleSentrySamplingContext::GetCustomSamplingContext() const
{
	return FAppleSentryConverters::VariantMapToUnreal(SamplingContext.customSamplingContext);
}

SentrySamplingContext* FAppleSentrySamplingContext::GetNativeObject()
{
	return SamplingContext;
}

#endif // !USE_SENTRY_NATIVE
