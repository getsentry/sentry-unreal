// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryTransaction;

class ISentryTransactionContext
{
public:
	virtual ~ISentryTransactionContext() = default;

	virtual FString GetName() const = 0;
	virtual FString GetOperation() const = 0;
};
