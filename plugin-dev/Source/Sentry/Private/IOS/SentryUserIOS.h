// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserInterface.h"

@class SentryUser;

class SentryUserIOS : public ISentryUser
{
public:
	SentryUserIOS();
	SentryUserIOS(SentryUser* user);
	virtual ~SentryUserIOS() override;

	SentryUser* GetNativeObject();

	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetId(const FString& id) override;
	virtual FString GetId() const override;
	virtual void SetUsername(const FString& username) override;
	virtual FString GetUsername() const override;
	virtual void SetIpAddress(const FString& ipAddress) override;
	virtual FString GetIpAddress() const override;
	virtual void SetData(const TMap<FString, FString>& data) override;
	virtual TMap<FString, FString> GetData() override;

private:
	SentryUser* UserIOS;
};
