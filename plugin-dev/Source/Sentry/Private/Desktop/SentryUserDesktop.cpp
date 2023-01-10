// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentryUserDesktop::SentryUserDesktop()
{
	UserDesktop = sentry_value_new_object();

	// Due to backwards compatibility concerns it is more reliable to initialize IP address explicitly with a specific value or "{{auto}}"
	sentry_value_set_by_key(UserDesktop, "ip_address", sentry_value_new_string("{{auto}}"));
}

SentryUserDesktop::SentryUserDesktop(sentry_value_t user)
{
	UserDesktop = user;
}

SentryUserDesktop::~SentryUserDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_value_t SentryUserDesktop::GetNativeObject()
{
	return UserDesktop;
}

void SentryUserDesktop::SetEmail(const FString& email)
{
	sentry_value_set_by_key(UserDesktop, "email", sentry_value_new_string(TCHAR_TO_ANSI(*email)));
}

FString SentryUserDesktop::GetEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(UserDesktop, "email");
	return FString(sentry_value_as_string(email));
}

void SentryUserDesktop::SetId(const FString& id)
{
	sentry_value_set_by_key(UserDesktop, "id", sentry_value_new_string(TCHAR_TO_ANSI(*id)));
}

FString SentryUserDesktop::GetId() const
{
	sentry_value_t id = sentry_value_get_by_key(UserDesktop, "id");
	return FString(sentry_value_as_string(id));
}

void SentryUserDesktop::SetUsername(const FString& username)
{
	sentry_value_set_by_key(UserDesktop, "username", sentry_value_new_string(TCHAR_TO_ANSI(*username)));
}

FString SentryUserDesktop::GetUsername() const
{
	sentry_value_t username = sentry_value_get_by_key(UserDesktop, "username");
	return FString(sentry_value_as_string(username));
}

void SentryUserDesktop::SetIpAddress(const FString& ipAddress)
{
	if(!ipAddress.IsEmpty())
	{
		sentry_value_set_by_key(UserDesktop, "ip_address", sentry_value_new_string(TCHAR_TO_ANSI(*ipAddress)));
	}
	else
	{
		sentry_value_set_by_key(UserDesktop, "ip_address", sentry_value_new_string("{{auto}}"));
	}
}

FString SentryUserDesktop::GetIpAddress() const
{
	sentry_value_t ip_address = sentry_value_get_by_key(UserDesktop, "ip_address");
	return FString(sentry_value_as_string(ip_address));
}

void SentryUserDesktop::SetData(const TMap<FString, FString>& data)
{
	sentry_value_set_by_key(UserDesktop, "data", SentryConvertorsDesktop::StringMapToNative(data));
}

TMap<FString, FString> SentryUserDesktop::GetData()
{
	sentry_value_t data = sentry_value_get_by_key(UserDesktop, "data");
	return SentryConvertorsDesktop::StringMapToUnreal(data);
}

#endif