// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

class ISentrySpan;

class ISentryTransaction
{
public:
	virtual ~ISentryTransaction() = default;

	virtual TSharedPtr<ISentrySpan> StartChildSpan(const FString& operation, const FString& desctiption) = 0;
	virtual TSharedPtr<ISentrySpan> StartChildSpanWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp) = 0;
	virtual void Finish() = 0;
	virtual void FinishWithTimestamp(int64 timestamp) = 0;
	virtual bool IsFinished() const = 0;
	virtual void SetName(const FString& name) = 0;
	virtual void SetTag(const FString& key, const FString& value) = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetData(const FString& key, const TMap<FString, FSentryVariant>& values) = 0;
	virtual void RemoveData(const FString& key) = 0;
	virtual void GetTrace(FString& name, FString& value) = 0;
};