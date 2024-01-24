// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USentryTransactionContext;

class ISentrySamplingContext
{
public:
	virtual ~ISentrySamplingContext() = default;

	virtual USentryTransactionContext* GetTransactionContext() const = 0;
	virtual TMap<FString, FString> GetCustomSamplingContext() const = 0;
};