// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

#include "Interface/SentryUserInterface.h"

class SentryUserAndroid : public ISentryUser
{
public:
	SentryUserAndroid();
	SentryUserAndroid(jobject user);
	virtual ~SentryUserAndroid() override;

	jobject GetNativeObject();

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
	jobject UserAndroid;
};
