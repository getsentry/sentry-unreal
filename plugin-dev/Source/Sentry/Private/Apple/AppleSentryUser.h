// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentryUserInterface.h"

@class SentryObjCUser;

class FAppleSentryUser : public ISentryUser
{
public:
	FAppleSentryUser();
	FAppleSentryUser(SentryObjCUser* user);
	virtual ~FAppleSentryUser() override;

	SentryObjCUser* GetNativeObject();

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
	SentryObjCUser* UserApple;
};

typedef FAppleSentryUser FPlatformSentryUser;

#endif // !USE_SENTRY_NATIVE
