// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserIOS.h"

#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryUserIOS::SentryUserIOS()
{
	UserIOS = [[SentryUser alloc] init];
}

SentryUserIOS::SentryUserIOS(SentryUser* user)
{
	UserIOS = user;
}

SentryUserIOS::~SentryUserIOS()
{
	// Put custom destructor logic here if needed
}

SentryUser* SentryUserIOS::GetNativeObject()
{
	return UserIOS;
}

void SentryUserIOS::SetEmail(const FString& email)
{
	UserIOS.email = email.GetNSString();
}

FString SentryUserIOS::GetEmail() const
{
	return FString(UserIOS.email);
}

void SentryUserIOS::SetId(const FString& id)
{
	UserIOS.userId = id.GetNSString();
}

FString SentryUserIOS::GetId() const
{
	return FString(UserIOS.userId);
}

void SentryUserIOS::SetUsername(const FString& username)
{
	UserIOS.username = username.GetNSString();
}

FString SentryUserIOS::GetUsername() const
{
	return FString(UserIOS.username);
}

void SentryUserIOS::SetIpAddress(const FString& ipAddress)
{
	UserIOS.ipAddress = ipAddress.GetNSString();
}

FString SentryUserIOS::GetIpAddress() const
{
	return FString(UserIOS.ipAddress);
}

void SentryUserIOS::SetData(const TMap<FString, FString>& data)
{
	UserIOS.data = SentryConvertorsIOS::StringMapToNative(data);
}

TMap<FString, FString> SentryUserIOS::GetData()
{
	return SentryConvertorsIOS::StringMapToUnreal(UserIOS.data);
}
