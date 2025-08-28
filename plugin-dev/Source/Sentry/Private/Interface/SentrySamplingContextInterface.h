// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryVariant.h"

class ISentryTransactionContext;

class ISentrySamplingContext
{
public:
	virtual ~ISentrySamplingContext() = default;

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const = 0;
	virtual TMap<FString, FSentryVariant> GetCustomSamplingContext() const = 0;
};