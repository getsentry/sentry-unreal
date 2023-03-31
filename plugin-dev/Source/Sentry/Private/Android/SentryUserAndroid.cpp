// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryUserAndroid::SentryUserAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::User, "()V")
{
	SetupClassMethods();
}

SentryUserAndroid::SentryUserAndroid(jobject user)
	: FSentryJavaObjectWrapper(SentryJavaClasses::User, user)
{
	SetupClassMethods();
}

void SentryUserAndroid::SetupClassMethods()
{
	SetEmailMethod = GetMethod("setEmail", "(Ljava/lang/String;)V");
	GetEmailMethod = GetMethod("getEmail", "()Ljava/lang/String;");
	SetIdMethod = GetMethod("setId", "(Ljava/lang/String;)V");
	GetIdMethod = GetMethod("getId", "()Ljava/lang/String;");
	SetUsernameMethod = GetMethod("setUsername", "(Ljava/lang/String;)V");
	GetUsernameMethod = GetMethod("getUsername", "()Ljava/lang/String;");
	SetIpAddressMethod = GetMethod("setIpAddress", "(Ljava/lang/String;)V");
	GetIpAddressMethod = GetMethod("getIpAddress", "()Ljava/lang/String;");
	SetDataMethod = GetMethod("setOthers", "(Ljava/util/Map;)V");
	GetDataMethod = GetMethod("getOthers", "()Ljava/util/Map;");
}

void SentryUserAndroid::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString SentryUserAndroid::GetEmail() const
{
	return CallMethod<FString>(GetEmailMethod);
}

void SentryUserAndroid::SetId(const FString& id)
{
	CallMethod<void>(SetIdMethod, *GetJString(id));
}

FString SentryUserAndroid::GetId() const
{
	return CallMethod<FString>(GetIdMethod);
}

void SentryUserAndroid::SetUsername(const FString& username)
{
	CallMethod<void>(SetUsernameMethod, *GetJString(username));
}

FString SentryUserAndroid::GetUsername() const
{
	return CallMethod<FString>(GetUsernameMethod);
}

void SentryUserAndroid::SetIpAddress(const FString& ipAddress)
{
	CallMethod<void>(SetIpAddressMethod, *GetJString(ipAddress));
}

FString SentryUserAndroid::GetIpAddress() const
{
	return CallMethod<FString>(GetIpAddressMethod);
}

void SentryUserAndroid::SetData(const TMap<FString, FString>& data)
{
	CallMethod<void>(SetDataMethod, SentryConvertorsAndroid::StringMapToNative(data)->GetJObject());
}

TMap<FString, FString> SentryUserAndroid::GetData()
{
	auto data = CallObjectMethod<jobject>(GetDataMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}