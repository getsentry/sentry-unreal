// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryLog
{

public:
	virtual ~ISentryLog() = default;

	virtual void SetBody(const FString& body) = 0;
	virtual FString GetBody() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;
};