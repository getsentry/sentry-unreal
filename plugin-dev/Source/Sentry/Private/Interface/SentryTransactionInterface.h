// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USentrySpan;

class ISentryTransaction
{
public:
	virtual ~ISentryTransaction() = default;

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) = 0;
	virtual void Finish() = 0;
};