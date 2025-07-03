// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySamplingContextInterface.h"

class FNullSentrySamplingContext final : public ISentrySamplingContext
{
public:
	virtual ~FNullSentrySamplingContext() override = default;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const override { return nullptr; }
	virtual TMap<FString, FString> GetCustomSamplingContext() const override { return {}; }
};

typedef FNullSentrySamplingContext FPlatformSentrySamplingContext;
