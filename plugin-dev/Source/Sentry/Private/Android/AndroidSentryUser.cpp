// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryUser.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryUser::FAndroidSentryUser()
	: FSentryJavaObjectWrapper(SentryJavaClasses::User, "()V")
{
	SetupClassMethods();
}

FAndroidSentryUser::FAndroidSentryUser(jobject user)
	: FSentryJavaObjectWrapper(SentryJavaClasses::User, user)
{
	SetupClassMethods();
}

void FAndroidSentryUser::SetupClassMethods()
{
	SetEmailMethod = GetMethod("setEmail", "(Ljava/lang/String;)V");
	GetEmailMethod = GetMethod("getEmail", "()Ljava/lang/String;");
	SetIdMethod = GetMethod("setId", "(Ljava/lang/String;)V");
	GetIdMethod = GetMethod("getId", "()Ljava/lang/String;");
	SetUsernameMethod = GetMethod("setUsername", "(Ljava/lang/String;)V");
	GetUsernameMethod = GetMethod("getUsername", "()Ljava/lang/String;");
	SetIpAddressMethod = GetMethod("setIpAddress", "(Ljava/lang/String;)V");
	GetIpAddressMethod = GetMethod("getIpAddress", "()Ljava/lang/String;");
	SetDataMethod = GetMethod("setData", "(Ljava/util/Map;)V");
	GetDataMethod = GetMethod("getData", "()Ljava/util/Map;");
}

void FAndroidSentryUser::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString FAndroidSentryUser::GetEmail() const
{
	return CallMethod<FString>(GetEmailMethod);
}

void FAndroidSentryUser::SetId(const FString& id)
{
	CallMethod<void>(SetIdMethod, *GetJString(id));
}

FString FAndroidSentryUser::GetId() const
{
	return CallMethod<FString>(GetIdMethod);
}

void FAndroidSentryUser::SetUsername(const FString& username)
{
	CallMethod<void>(SetUsernameMethod, *GetJString(username));
}

FString FAndroidSentryUser::GetUsername() const
{
	return CallMethod<FString>(GetUsernameMethod);
}

void FAndroidSentryUser::SetIpAddress(const FString& ipAddress)
{
	CallMethod<void>(SetIpAddressMethod, *GetJString(ipAddress));
}

FString FAndroidSentryUser::GetIpAddress() const
{
	return CallMethod<FString>(GetIpAddressMethod);
}

void FAndroidSentryUser::SetData(const TMap<FString, FString>& data)
{
	CallMethod<void>(SetDataMethod, FAndroidSentryConverters::StringMapToNative(data)->GetJObject());
}

TMap<FString, FString> FAndroidSentryUser::GetData()
{
	auto data = CallObjectMethod<jobject>(GetDataMethod);
	return FAndroidSentryConverters::StringMapToUnreal(*data);
}