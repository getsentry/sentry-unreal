// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

#include "Android/AndroidApplication.h"

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid()
	: FJavaClassObject(GetClassName(), "()V")
	, SetMessageMethod(GetClassMethod("setMessage", "(Ljava/lang/String;)V"))
	, GetMessageMethod(GetClassMethod("getMessage", "()Ljava/lang/String;"))
	, SetTypeMethod(GetClassMethod("setType", "(Ljava/lang/String;)V"))
	, GetTypeMethod(GetClassMethod("getType", "()Ljava/lang/String;"))
	, SetCategoryMethod(GetClassMethod("setCategory", "(Ljava/lang/String;)V"))
	, GetCategoryMethod(GetClassMethod("getCategory", "()Ljava/lang/String;"))
	, SetDataMethod(GetClassMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V"))
	, GetDataMethod(GetClassMethod("getData", "()Ljava/util/Map;"))
	, SetLevelMethod(GetClassMethod("setLevel", "(Lio/sentry/SentryLevel;)V"))
	, GetLevelMethod(GetClassMethod("getLevel", "()Lio/sentry/SentryLevel;"))
{
}

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid(jobject breadcrumb)
	: SentryBreadcrumbAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if(Env->IsInstanceOf(breadcrumb, Class))
	{
		// Remove default object's global reference before re-assigning Object field
		Env->DeleteGlobalRef(Object);
		Object = Env->NewGlobalRef(breadcrumb);
	}
}

FName SentryBreadcrumbAndroid::GetClassName()
{
	return FName("io/sentry/Breadcrumb");
}

void SentryBreadcrumbAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

FString SentryBreadcrumbAndroid::GetMessage() const
{
	return const_cast<SentryBreadcrumbAndroid*>(this)->CallMethod<FString>(GetMessageMethod);
}

void SentryBreadcrumbAndroid::SetType(const FString& type)
{
	CallMethod<void>(SetTypeMethod, *GetJString(type));
}

FString SentryBreadcrumbAndroid::GetType() const
{
	return const_cast<SentryBreadcrumbAndroid*>(this)->CallMethod<FString>(GetTypeMethod);
}

void SentryBreadcrumbAndroid::SetCategory(const FString& category)
{
	CallMethod<void>(SetCategoryMethod, *GetJString(category));
}

FString SentryBreadcrumbAndroid::GetCategory() const
{
	return const_cast<SentryBreadcrumbAndroid*>(this)->CallMethod<FString>(GetCategoryMethod);
}

void SentryBreadcrumbAndroid::SetData(const TMap<FString, FString>& data)
{
	for (const auto& dataItem : data)
	{
		CallMethod<void>(SetDataMethod, *GetJString(dataItem.Key), *GetJString(dataItem.Value));
	}
}

TMap<FString, FString> SentryBreadcrumbAndroid::GetData() const
{
	auto data = NewSentryScopedJavaObject(const_cast<SentryBreadcrumbAndroid*>(this)->CallMethod<jobject>(GetDataMethod));
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}

void SentryBreadcrumbAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryBreadcrumbAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(const_cast<SentryBreadcrumbAndroid*>(this)->CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}
