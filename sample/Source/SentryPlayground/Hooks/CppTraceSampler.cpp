// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppTraceSampler.h"

#include "SentrySamplingContext.h"
#include "SentryTransactionContext.h"

bool UCppTraceSampler::Sample_Implementation(USentrySamplingContext* samplingContext, float& samplingValue)
{
	USentryTransactionContext* TransactionContext = samplingContext->GetTransactionContext();

	if (TransactionContext && TransactionContext->GetName() == TEXT("integration.tracing.test"))
	{
		samplingValue = 1.0f;
		return true;
	}

	// Discard transactions not matching the expected name
	samplingValue = 0.0f;
	return true;
}
