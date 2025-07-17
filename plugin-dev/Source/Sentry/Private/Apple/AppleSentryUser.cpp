// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryUser.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryUser::FAppleSentryUser()
{
	UserApple = [[SENTRY_APPLE_CLASS(SentryUser) alloc] init];
}

FAppleSentryUser::FAppleSentryUser(SentryUser* user)
{
	UserApple = user;
}

FAppleSentryUser::~FAppleSentryUser()
{
	// Put custom destructor logic here if needed
}

SentryUser* FAppleSentryUser::GetNativeObject()
{
	return UserApple;
}

void FAppleSentryUser::SetEmail(const FString& email)
{
	UserApple.email = email.GetNSString();
}

FString FAppleSentryUser::GetEmail() const
{
	return FString(UserApple.email);
}

void FAppleSentryUser::SetId(const FString& id)
{
	UserApple.userId = id.GetNSString();
}

FString FAppleSentryUser::GetId() const
{
	return FString(UserApple.userId);
}

void FAppleSentryUser::SetUsername(const FString& username)
{
	UserApple.username = username.GetNSString();
}

FString FAppleSentryUser::GetUsername() const
{
	return FString(UserApple.username);
}

void FAppleSentryUser::SetIpAddress(const FString& ipAddress)
{
	UserApple.ipAddress = ipAddress.GetNSString();
}

FString FAppleSentryUser::GetIpAddress() const
{
	return FString(UserApple.ipAddress);
}

void FAppleSentryUser::SetData(const TMap<FString, FString>& data)
{
	UserApple.data = FAppleSentryConverters::StringMapToNative(data);
}

TMap<FString, FString> FAppleSentryUser::GetData()
{
	return FAppleSentryConverters::StringMapToUnreal(UserApple.data);
}
