// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::Breadcrumb, "()V")
{
	SetupClassMethods();
}

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid(jobject breadcrumb)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Breadcrumb, breadcrumb)
{
	SetupClassMethods();
}

void SentryBreadcrumbAndroid::SetupClassMethods()
{
	SetMessageMethod = GetMethod("setMessage", "(Ljava/lang/String;)V");
	GetMessageMethod = GetMethod("getMessage", "()Ljava/lang/String;");
	SetTypeMethod = GetMethod("setType", "(Ljava/lang/String;)V");
	GetTypeMethod = GetMethod("getType", "()Ljava/lang/String;");
	SetCategoryMethod = GetMethod("setCategory", "(Ljava/lang/String;)V");
	GetCategoryMethod = GetMethod("getCategory", "()Ljava/lang/String;");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	GetDataMethod = GetMethod("getData", "()Ljava/util/Map;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
}

void SentryBreadcrumbAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

FString SentryBreadcrumbAndroid::GetMessage() const
{
	return CallMethod<FString>(GetMessageMethod);
}

void SentryBreadcrumbAndroid::SetType(const FString& type)
{
	CallMethod<void>(SetTypeMethod, *GetJString(type));
}

FString SentryBreadcrumbAndroid::GetType() const
{
	return CallMethod<FString>(GetTypeMethod);
}

void SentryBreadcrumbAndroid::SetCategory(const FString& category)
{
	CallMethod<void>(SetCategoryMethod, *GetJString(category));
}

FString SentryBreadcrumbAndroid::GetCategory() const
{
	return CallMethod<FString>(GetCategoryMethod);
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
	auto data = CallObjectMethod<jobject>(GetDataMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}

void SentryBreadcrumbAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel SentryBreadcrumbAndroid::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}