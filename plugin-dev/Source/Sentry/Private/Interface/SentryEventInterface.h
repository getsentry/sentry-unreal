// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryId;

class ISentryEvent
{
public:
	virtual ~ISentryEvent() = default;

	virtual TSharedPtr<ISentryId> GetId() const = 0;
	virtual void SetMessage(const FString& message) = 0;
	virtual FString GetMessage() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) = 0;
	virtual TArray<FString> GetFingerprint() = 0;
	virtual bool IsCrash() const = 0;
	virtual bool IsAnr() const = 0;
};
