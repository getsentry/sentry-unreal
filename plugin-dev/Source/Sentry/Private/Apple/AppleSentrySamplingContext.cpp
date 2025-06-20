// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySamplingContext.h"
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

TMap<FString, FString> FAppleSentrySamplingContext::GetCustomSamplingContext() const
{
	return FAppleSentryConverters::StringMapToUnreal(SamplingContext.customSamplingContext);
}

SentrySamplingContext* FAppleSentrySamplingContext::GetNativeObject()
{
	return SamplingContext;
}
