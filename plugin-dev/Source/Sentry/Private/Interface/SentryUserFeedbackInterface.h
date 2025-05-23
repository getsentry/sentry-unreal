// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryUserFeedback
{
public:
	virtual ~ISentryUserFeedback() = default;

	virtual void SetName(const FString& name) = 0;
	virtual FString GetName() const = 0;
	virtual void SetEmail(const FString& email) = 0;
	virtual FString GetEmail() const = 0;
	virtual void SetComment(const FString& comment) = 0;
	virtual FString GetComment() const = 0;
};