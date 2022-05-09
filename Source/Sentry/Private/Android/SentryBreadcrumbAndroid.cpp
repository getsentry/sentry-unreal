// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbAndroid.h"

#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass breadcrumbClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/Breadcrumb");
	jmethodID breadcrumbCtor = Env->GetMethodID(breadcrumbClass, "<init>", "()V");
	jobject breadcrumbObject= Env->NewObject(breadcrumbClass, breadcrumbCtor);
	BreadcrumbAndroid = Env->NewGlobalRef(breadcrumbObject);
}

SentryBreadcrumbAndroid::SentryBreadcrumbAndroid(jobject breadcrumb)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	BreadcrumbAndroid = Env->NewGlobalRef(breadcrumb);
}

SentryBreadcrumbAndroid::~SentryBreadcrumbAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(BreadcrumbAndroid);
}

jobject SentryBreadcrumbAndroid::GetNativeObject()
{
	return BreadcrumbAndroid;
}

void SentryBreadcrumbAndroid::SetMessage(const FString& message)
{
	SentryMethodCallAndroid::CallVoidMethod(BreadcrumbAndroid, "setMessage", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(message));
}

FString SentryBreadcrumbAndroid::GetMessage() const
{
	return SentryMethodCallAndroid::CallStringMethod(BreadcrumbAndroid, "getMessage", "()Ljava/lang/String;");
}

void SentryBreadcrumbAndroid::SetType(const FString& type)
{
	SentryMethodCallAndroid::CallVoidMethod(BreadcrumbAndroid, "setType", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(type));
}

FString SentryBreadcrumbAndroid::GetType() const
{
	return SentryMethodCallAndroid::CallStringMethod(BreadcrumbAndroid, "getType", "()Ljava/lang/String;");
}

void SentryBreadcrumbAndroid::SetCategory(const FString& category)
{
	SentryMethodCallAndroid::CallVoidMethod(BreadcrumbAndroid, "setCategory", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(category));
}

FString SentryBreadcrumbAndroid::GetCategory() const
{
	return SentryMethodCallAndroid::CallStringMethod(BreadcrumbAndroid, "getCategory", "()Ljava/lang/String;");
}

void SentryBreadcrumbAndroid::SetData(const TMap<FString, FString>& data)
{
	for (const auto& dataItem : data)
	{
		SentryMethodCallAndroid::CallVoidMethod(BreadcrumbAndroid, "setData", "(Ljava/lang/String;Ljava/lang/Object;)V",
			SentryConvertorsAndroid::StringToNative(dataItem.Key), SentryConvertorsAndroid::StringToNative(dataItem.Value));
	}
}

TMap<FString, FString> SentryBreadcrumbAndroid::GetData() const
{
	jobject data = SentryMethodCallAndroid::CallObjectMethod(BreadcrumbAndroid, "getData", "()Ljava/util/Map;");
	return SentryConvertorsAndroid::StringMapToUnreal(data);
}

void SentryBreadcrumbAndroid::SetLevel(ESentryLevel level)
{
	SentryMethodCallAndroid::CallVoidMethod(BreadcrumbAndroid, "setLevel", "(Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryBreadcrumbAndroid::GetLevel() const
{
	jobject level = SentryMethodCallAndroid::CallObjectMethod(BreadcrumbAndroid, "getLevel", "()Lio/sentry/SentryLevel;");
	return SentryConvertorsAndroid::SentryLevelToUnreal(level);
}
