// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryTransaction;

class ISentryTransactionContext
{
public:
	virtual ~ISentryTransactionContext() = default;

	virtual TSharedPtr<ISentryTransaction> StartTransaction() const = 0;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithTimestamp(int64 Timestamp) const = 0;
	virtual FString GetName() const = 0;
	virtual FString GetOperation() const = 0;
};