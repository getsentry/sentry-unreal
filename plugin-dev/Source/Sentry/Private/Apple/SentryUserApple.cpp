// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryUserApple::SentryUserApple()
{
	UserApple = [[SENTRY_APPLE_CLASS(SentryUser) alloc] init];
}

SentryUserApple::SentryUserApple(SentryUser* user)
{
	UserApple = user;
}

SentryUserApple::~SentryUserApple()
{
	// Put custom destructor logic here if needed
}

SentryUser* SentryUserApple::GetNativeObject()
{
	return UserApple;
}

void SentryUserApple::SetEmail(const FString& email)
{
	UserApple.email = email.GetNSString();
}

FString SentryUserApple::GetEmail() const
{
	return FString(UserApple.email);
}

void SentryUserApple::SetId(const FString& id)
{
	UserApple.userId = id.GetNSString();
}

FString SentryUserApple::GetId() const
{
	return FString(UserApple.userId);
}

void SentryUserApple::SetUsername(const FString& username)
{
	UserApple.username = username.GetNSString();
}

FString SentryUserApple::GetUsername() const
{
	return FString(UserApple.username);
}

void SentryUserApple::SetIpAddress(const FString& ipAddress)
{
	UserApple.ipAddress = ipAddress.GetNSString();
}

FString SentryUserApple::GetIpAddress() const
{
	return FString(UserApple.ipAddress);
}

void SentryUserApple::SetData(const TMap<FString, FString>& data)
{
	UserApple.data = SentryConvertorsApple::StringMapToNative(data);
}

TMap<FString, FString> SentryUserApple::GetData()
{
	return SentryConvertorsApple::StringMapToUnreal(UserApple.data);
}
