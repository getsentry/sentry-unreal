// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryUser
{
public:
	virtual ~ISentryUser() = default;

	virtual void SetEmail(const FString& email) = 0;
	virtual FString GetEmail() const = 0;
	virtual void SetId(const FString& id) = 0;
	virtual FString GetId() const = 0;
	virtual void SetUsername(const FString& username) = 0;
	virtual FString GetUsername() const = 0;
	virtual void SetIpAddress(const FString& ipAddress) = 0;
	virtual FString GetIpAddress() const = 0;
	virtual void SetData(const TMap<FString, FString>& data) = 0;
	virtual TMap<FString, FString> GetData() = 0;
};