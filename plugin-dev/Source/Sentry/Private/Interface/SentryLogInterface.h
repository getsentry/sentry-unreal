// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

class ISentryLog
{

public:
	virtual ~ISentryLog() = default;

	virtual void SetBody(const FString& body) = 0;
	virtual FString GetBody() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) = 0;
	virtual FSentryVariant GetAttribute(const FString& key) const = 0;
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const = 0;
	virtual void RemoveAttribute(const FString& key) = 0;
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) = 0;
};