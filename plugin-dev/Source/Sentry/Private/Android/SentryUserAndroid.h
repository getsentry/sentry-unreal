// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class SentryUserAndroid : public ISentryUser, public FSentryJavaClassWrapper
{
public:
	SentryUserAndroid();
	SentryUserAndroid(jobject user);

	void SetupClassMethods();

	static FName GetClassName();

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
	FJavaClassMethod SetEmailMethod;
	FJavaClassMethod GetEmailMethod;
	FJavaClassMethod SetIdMethod;
	FJavaClassMethod GetIdMethod;
	FJavaClassMethod SetUsernameMethod;
	FJavaClassMethod GetUsernameMethod;
	FJavaClassMethod SetIpAddressMethod;
	FJavaClassMethod GetIpAddressMethod;
	FJavaClassMethod SetDataMethod;
	FJavaClassMethod GetDataMethod;
};
