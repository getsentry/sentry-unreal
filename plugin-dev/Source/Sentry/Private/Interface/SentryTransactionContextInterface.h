// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryTransactionContext
{
public:
	virtual ~ISentryTransactionContext() = default;

	virtual FString GetName() const = 0;
	virtual FString GetOperation() const = 0;
};