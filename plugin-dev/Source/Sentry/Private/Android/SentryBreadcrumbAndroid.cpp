// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid()
	: FSentryJavaClassWrapper(GetClassName(), "()V")
{
	SetupClassMethods();
}

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid(jobject breadcrumb)
	: FSentryJavaClassWrapper(GetClassName(), breadcrumb)
{
	SetupClassMethods();
}

void SentryBreadcrumbAndroid::SetupClassMethods()
{
	SetMessageMethod = GetClassMethod("setMessage", "(Ljava/lang/String;)V");
	GetMessageMethod = GetClassMethod("getMessage", "()Ljava/lang/String;");
	SetTypeMethod = GetClassMethod("setType", "(Ljava/lang/String;)V");
	GetTypeMethod = GetClassMethod("getType", "()Ljava/lang/String;");
	SetCategoryMethod = GetClassMethod("setCategory", "(Ljava/lang/String;)V");
	GetCategoryMethod = GetClassMethod("getCategory", "()Ljava/lang/String;");
	SetDataMethod = GetClassMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	GetDataMethod = GetClassMethod("getData", "()Ljava/util/Map;");
	SetLevelMethod = GetClassMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetClassMethod("getLevel", "()Lio/sentry/SentryLevel;");
}

FName SentryBreadcrumbAndroid::GetClassName()
{
	return FName("io/sentry/Breadcrumb");
}

void SentryBreadcrumbAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, *FJavaClassObject::GetJString(message));
}

FString SentryBreadcrumbAndroid::GetMessage() const
{
	return CallMethod<FString>(GetMessageMethod);
}

void SentryBreadcrumbAndroid::SetType(const FString& type)
{
	CallMethod<void>(SetTypeMethod, *FJavaClassObject::GetJString(type));
}

FString SentryBreadcrumbAndroid::GetType() const
{
	return CallMethod<FString>(GetTypeMethod);
}

void SentryBreadcrumbAndroid::SetCategory(const FString& category)
{
	CallMethod<void>(SetCategoryMethod, *FJavaClassObject::GetJString(category));
}

FString SentryBreadcrumbAndroid::GetCategory() const
{
	return CallMethod<FString>(GetCategoryMethod);
}

void SentryBreadcrumbAndroid::SetData(const TMap<FString, FString>& data)
{
	for (const auto& dataItem : data)
	{
		CallMethod<void>(SetDataMethod, *FJavaClassObject::GetJString(dataItem.Key), *FJavaClassObject::GetJString(dataItem.Value));
	}
}

TMap<FString, FString> SentryBreadcrumbAndroid::GetData() const
{
	auto data = NewSentryScopedJavaObject(CallMethod<jobject>(GetDataMethod));
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}

void SentryBreadcrumbAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryBreadcrumbAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}