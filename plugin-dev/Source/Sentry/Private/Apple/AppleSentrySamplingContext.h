// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentrySamplingContextInterface.h"

@class SentryObjCSamplingContext;

class FAppleSentrySamplingContext : public ISentrySamplingContext
{
public:
	FAppleSentrySamplingContext(SentryObjCSamplingContext* context);
	virtual ~FAppleSentrySamplingContext() override;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const override;
	virtual TMap<FString, FSentryVariant> GetCustomSamplingContext() const override;

	SentryObjCSamplingContext* GetNativeObject();

private:
	SentryObjCSamplingContext* SamplingContext;
};

typedef FAppleSentrySamplingContext FPlatformSentrySamplingContext;

#endif // !USE_SENTRY_NATIVE
