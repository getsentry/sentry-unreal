// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "AppleSentryUser.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FSentryUserApple::FSentryUserApple()
{
	UserApple = [[SENTRY_APPLE_CLASS(SentryUser) alloc] init];
}

FSentryUserApple::FSentryUserApple(SentryUser* user)
{
	UserApple = user;
}

FSentryUserApple::~FSentryUserApple()
{
	// Put custom destructor logic here if needed
}

SentryUser* FSentryUserApple::GetNativeObject()
{
	return UserApple;
}

void FSentryUserApple::SetEmail(const FString& email)
{
	UserApple.email = email.GetNSString();
}

FString FSentryUserApple::GetEmail() const
{
	return FString(UserApple.email);
}

void FSentryUserApple::SetId(const FString& id)
{
	UserApple.userId = id.GetNSString();
}

FString FSentryUserApple::GetId() const
{
	return FString(UserApple.userId);
}

void FSentryUserApple::SetUsername(const FString& username)
{
	UserApple.username = username.GetNSString();
}

FString FSentryUserApple::GetUsername() const
{
	return FString(UserApple.username);
}

void FSentryUserApple::SetIpAddress(const FString& ipAddress)
{
	UserApple.ipAddress = ipAddress.GetNSString();
}

FString FSentryUserApple::GetIpAddress() const
{
	return FString(UserApple.ipAddress);
}

void FSentryUserApple::SetData(const TMap<FString, FString>& data)
{
	UserApple.data = FAppleSentryConverters::StringMapToNative(data);
}

TMap<FString, FString> FSentryUserApple::GetData()
{
	return FAppleSentryConverters::StringMapToUnreal(UserApple.data);
}
