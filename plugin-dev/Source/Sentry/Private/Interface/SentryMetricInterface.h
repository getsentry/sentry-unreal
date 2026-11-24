// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

class ISentryMetric
{

public:
	virtual ~ISentryMetric() = default;

	virtual void SetName(const FString& name) = 0;
	virtual FString GetName() const = 0;
	virtual void SetType(const FString& type) = 0;
	virtual FString GetType() const = 0;
	virtual void SetValue(float value) = 0;
	virtual float GetValue() const = 0;
	virtual void SetUnit(const FString& unit) = 0;
	virtual FString GetUnit() const = 0;

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) = 0;
	virtual FSentryVariant GetAttribute(const FString& key) const = 0;
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const = 0;
	virtual void RemoveAttribute(const FString& key) = 0;
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) = 0;
};
