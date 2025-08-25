// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryFeedback
{
public:
	virtual ~ISentryFeedback() = default;

	virtual FString GetMessage() const = 0;
	virtual void SetName(const FString& name) = 0;
	virtual FString GetName() const = 0;
	virtual void SetContactEmail(const FString& email) = 0;
	virtual FString GetContactEmail() const = 0;
	virtual void SetAssociatedEvent(const FString& eventId) = 0;
	virtual FString GetAssociatedEvent() const = 0;
};