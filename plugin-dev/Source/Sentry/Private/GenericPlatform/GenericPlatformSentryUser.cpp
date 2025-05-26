// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryUser.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryUser::FGenericPlatformSentryUser()
{
	User = sentry_value_new_object();
}

FGenericPlatformSentryUser::FGenericPlatformSentryUser(sentry_value_t user)
{
	User = user;
}

FGenericPlatformSentryUser::~FGenericPlatformSentryUser()
{
	// Put custom destructor logic here if needed
}

sentry_value_t FGenericPlatformSentryUser::GetNativeObject()
{
	return User;
}

void FGenericPlatformSentryUser::SetEmail(const FString& email)
{
	sentry_value_set_by_key(User, "email", sentry_value_new_string(TCHAR_TO_ANSI(*email)));
}

FString FGenericPlatformSentryUser::GetEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(User, "email");
	return FString(sentry_value_as_string(email));
}

void FGenericPlatformSentryUser::SetId(const FString& id)
{
	sentry_value_set_by_key(User, "id", sentry_value_new_string(TCHAR_TO_ANSI(*id)));
}

FString FGenericPlatformSentryUser::GetId() const
{
	sentry_value_t id = sentry_value_get_by_key(User, "id");
	return FString(sentry_value_as_string(id));
}

void FGenericPlatformSentryUser::SetUsername(const FString& username)
{
	sentry_value_set_by_key(User, "username", sentry_value_new_string(TCHAR_TO_ANSI(*username)));
}

FString FGenericPlatformSentryUser::GetUsername() const
{
	sentry_value_t username = sentry_value_get_by_key(User, "username");
	return FString(sentry_value_as_string(username));
}

void FGenericPlatformSentryUser::SetIpAddress(const FString& ipAddress)
{
	if (!ipAddress.IsEmpty())
	{
		sentry_value_set_by_key(User, "ip_address", sentry_value_new_string(TCHAR_TO_ANSI(*ipAddress)));
	}
	else
	{
		sentry_value_set_by_key(User, "ip_address", sentry_value_new_null());
	}
}

FString FGenericPlatformSentryUser::GetIpAddress() const
{
	sentry_value_t ip_address = sentry_value_get_by_key(User, "ip_address");
	return FString(sentry_value_as_string(ip_address));
}

void FGenericPlatformSentryUser::SetData(const TMap<FString, FString>& data)
{
	sentry_value_set_by_key(User, "data", FGenericPlatformSentryConverters::StringMapToNative(data));
}

TMap<FString, FString> FGenericPlatformSentryUser::GetData()
{
	sentry_value_t data = sentry_value_get_by_key(User, "data");
	return FGenericPlatformSentryConverters::StringMapToUnreal(data);
}

#endif
