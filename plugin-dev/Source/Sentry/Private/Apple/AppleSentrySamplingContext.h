// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySamplingContextInterface.h"

@class SentrySamplingContext;

class FAppleSentrySamplingContext : public ISentrySamplingContext
{
public:
	FAppleSentrySamplingContext(SentrySamplingContext* context);
	virtual ~FAppleSentrySamplingContext() override;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const override;
	virtual TMap<FString, FString> GetCustomSamplingContext() const override;

	SentrySamplingContext* GetNativeObject();

private:
	SentrySamplingContext* SamplingContext;
};

typedef FAppleSentrySamplingContext FPlatformSentrySamplingContext;
