// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryBreadcrumb.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryBreadcrumb::FAndroidSentryBreadcrumb()
	: FSentryJavaObjectWrapper(SentryJavaClasses::Breadcrumb, "()V")
{
	SetupClassMethods();
}

FAndroidSentryBreadcrumb::FAndroidSentryBreadcrumb(jobject breadcrumb)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Breadcrumb, breadcrumb)
{
	SetupClassMethods();
}

void FAndroidSentryBreadcrumb::SetupClassMethods()
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

void FAndroidSentryBreadcrumb::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

FString FAndroidSentryBreadcrumb::GetMessage() const
{
	return CallMethod<FString>(GetMessageMethod);
}

void FAndroidSentryBreadcrumb::SetType(const FString& type)
{
	CallMethod<void>(SetTypeMethod, *GetJString(type));
}

FString FAndroidSentryBreadcrumb::GetType() const
{
	return CallMethod<FString>(GetTypeMethod);
}

void FAndroidSentryBreadcrumb::SetCategory(const FString& category)
{
	CallMethod<void>(SetCategoryMethod, *GetJString(category));
}

FString FAndroidSentryBreadcrumb::GetCategory() const
{
	return CallMethod<FString>(GetCategoryMethod);
}

void FAndroidSentryBreadcrumb::SetData(const TMap<FString, FSentryVariant>& data)
{
	for (const auto& dataItem : data)
	{
		CallMethod<void>(SetDataMethod, *GetJString(dataItem.Key), FAndroidSentryConverters::VariantToNative(dataItem.Value)->GetJObject());
	}
}

TMap<FString, FSentryVariant> FAndroidSentryBreadcrumb::GetData() const
{
	auto data = CallObjectMethod<jobject>(GetDataMethod);
	return FAndroidSentryConverters::VariantMapToUnreal(*data);
}

void FAndroidSentryBreadcrumb::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel FAndroidSentryBreadcrumb::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return FAndroidSentryConverters::SentryLevelToUnreal(*level);
}