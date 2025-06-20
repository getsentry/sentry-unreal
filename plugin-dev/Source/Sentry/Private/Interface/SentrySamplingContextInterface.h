// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryTransactionContext;

class ISentrySamplingContext
{
public:
	virtual ~ISentrySamplingContext() = default;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const = 0;
	virtual TMap<FString, FString> GetCustomSamplingContext() const = 0;
};