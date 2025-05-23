// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryUser : public ISentryUser, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryUser();
	FAndroidSentryUser(jobject user);

	void SetupClassMethods();

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
	FSentryJavaMethod SetEmailMethod;
	FSentryJavaMethod GetEmailMethod;
	FSentryJavaMethod SetIdMethod;
	FSentryJavaMethod GetIdMethod;
	FSentryJavaMethod SetUsernameMethod;
	FSentryJavaMethod GetUsernameMethod;
	FSentryJavaMethod SetIpAddressMethod;
	FSentryJavaMethod GetIpAddressMethod;
	FSentryJavaMethod SetDataMethod;
	FSentryJavaMethod GetDataMethod;
};

typedef FAndroidSentryUser FPlatformSentryUser;
