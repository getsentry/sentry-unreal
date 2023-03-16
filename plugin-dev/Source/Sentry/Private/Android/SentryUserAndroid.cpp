// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryUserAndroid::SentryUserAndroid()
	: FJavaClassObject(GetClassName(), "()V")
	, SetEmailMethod(GetClassMethod("setEmail", "(Ljava/lang/String;)V"))
	, GetEmailMethod(GetClassMethod("getEmail", "()Ljava/lang/String;"))
	, SetIdMethod(GetClassMethod("setId", "(Ljava/lang/String;)V"))
	, GetIdMethod(GetClassMethod("getId", "()Ljava/lang/String;"))
	, SetUsernameMethod(GetClassMethod("setUsername", "(Ljava/lang/String;)V"))
	, GetUsernameMethod(GetClassMethod("getUsername", "()Ljava/lang/String;"))
	, SetIpAddressMethod(GetClassMethod("setIpAddress", "(Ljava/lang/String;)V"))
	, GetIpAddressMethod(GetClassMethod("getIpAddress", "()Ljava/lang/String;"))
	, SetDataMethod(GetClassMethod("setOthers", "(Ljava/util/Map;)V"))
	, GetDataMethod(GetClassMethod("getOthers", "()Ljava/util/Map;"))
{
}

SentryUserAndroid::SentryUserAndroid(jobject user)
	: SentryUserAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if(Env->IsInstanceOf(user, Class))
	{
		// Remove default object's global reference before re-assigning Object field
		Env->DeleteGlobalRef(Object);
		Object = Env->NewGlobalRef(user);
	}
}

FName SentryUserAndroid::GetClassName()
{
	return FName("io/sentry/protocol/User");
}

void SentryUserAndroid::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString SentryUserAndroid::GetEmail() const
{
	return const_cast<SentryUserAndroid*>(this)->CallMethod<FString>(GetEmailMethod);
}

void SentryUserAndroid::SetId(const FString& id)
{
	CallMethod<void>(SetIdMethod, *GetJString(id));
}

FString SentryUserAndroid::GetId() const
{
	return const_cast<SentryUserAndroid*>(this)->CallMethod<FString>(GetIdMethod);
}

void SentryUserAndroid::SetUsername(const FString& username)
{
	CallMethod<void>(SetUsernameMethod, *GetJString(username));
}

FString SentryUserAndroid::GetUsername() const
{
	return const_cast<SentryUserAndroid*>(this)->CallMethod<FString>(GetUsernameMethod);
}

void SentryUserAndroid::SetIpAddress(const FString& ipAddress)
{
	CallMethod<void>(SetIpAddressMethod, *GetJString(ipAddress));
}

FString SentryUserAndroid::GetIpAddress() const
{
	return const_cast<SentryUserAndroid*>(this)->CallMethod<FString>(GetIpAddressMethod);
}

void SentryUserAndroid::SetData(const TMap<FString, FString>& data)
{
	CallMethod<void>(SetDataMethod, SentryConvertorsAndroid::StringMapToNative(data));
}

TMap<FString, FString> SentryUserAndroid::GetData()
{
	auto data = NewSentryScopedJavaObject(const_cast<SentryUserAndroid*>(this)->CallMethod<jobject>(GetDataMethod));
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}