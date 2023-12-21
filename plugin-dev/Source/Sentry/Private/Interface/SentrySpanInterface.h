// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentrySpan
{
public:
	virtual ~ISentrySpan() = default;

	virtual void Finish() = 0;
	virtual bool IsFinished() const = 0;
	virtual void SetTag(const FString& key, const FString& value) = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) = 0;
	virtual void RemoveData(const FString& key) = 0;
};