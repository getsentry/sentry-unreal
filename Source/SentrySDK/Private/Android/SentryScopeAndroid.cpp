// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

const ANSICHAR* SentryScopeAndroid::SentryScopeJavaClassName = "com/sentry/unreal/SentryScopeJava";

SentryScopeAndroid::~SentryScopeAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(ScopeAndroid);
}

void SentryScopeAndroid::Init()
{
	// TODO Some default native object initialization is required
}

void SentryScopeAndroid::InitWithNativeObject(jobject scope)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	ScopeAndroid = Env->NewGlobalRef(scope);
}

jobject SentryScopeAndroid::GetNativeObject()
{
	return ScopeAndroid;
}

void SentryScopeAndroid::SetTagValue(const FString& key, const FString& value)
{
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "setTag", "(Ljava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(key), SentryConvertorsAndroid::StringToNative(value));
}

void SentryScopeAndroid::RemoveTag(const FString& key)
{
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "removeTag", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(key));
}

void SentryScopeAndroid::SetTags(const TMap<FString, FString>& tags)
{
	for (const auto& tag : tags)
	{
		SetTagValue(tag.Key, tag.Value);
	}
}

void SentryScopeAndroid::SetDist(const FString& dist)
{
	SetTagValue("dist", dist);
}

void SentryScopeAndroid::SetEnvironment(const FString& environment)
{
	SetTagValue("environment", environment);
}

void SentryScopeAndroid::SetFingerprint(const TArray<FString>& fingerprint)
{
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "setFingerprint", "(Ljava/util/List;)V",
		SentryConvertorsAndroid::StringArrayToNative(fingerprint));
}

void SentryScopeAndroid::SetLevel(ESentryLevel level)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryScopeJavaClassName, "setScopeLevel", "(Lio/sentry/Scope;I)V",
		ScopeAndroid, static_cast<int>(level));
}

void SentryScopeAndroid::Clear()
{
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "clear", "()V");
}
