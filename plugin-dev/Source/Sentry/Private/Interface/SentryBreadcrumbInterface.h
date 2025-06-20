// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

class ISentryBreadcrumb
{

public:
	virtual ~ISentryBreadcrumb() = default;

	virtual void SetMessage(const FString& message) = 0;
	virtual FString GetMessage() const = 0;
	virtual void SetType(const FString& type) = 0;
	virtual FString GetType() const = 0;
	virtual void SetCategory(const FString& category) = 0;
	virtual FString GetCategory() const = 0;
	virtual void SetData(const TMap<FString, FSentryVariant>& data) = 0;
	virtual TMap<FString, FSentryVariant> GetData() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;
};