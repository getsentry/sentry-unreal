// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryScopeAndroid::SentryScopeAndroid()
{
	jobject hub = SentryMethodCallAndroid::CallStaticObjectMethod("io/sentry/Sentry", "getCurrentHub", "()Lio/sentry/IHub;");
	jobject options = SentryMethodCallAndroid::CallObjectMethod(hub, "getOptions", "()Lio/sentry/SentryOptions;");

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass scopeClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/Scope");
	jmethodID scopeCtor = Env->GetMethodID(scopeClass, "<init>", "(Lio/sentry/SentryOptions;)V");
	jobject scopeObject= Env->NewObject(scopeClass, scopeCtor, options);
	ScopeAndroid = Env->NewGlobalRef(scopeObject);
}

SentryScopeAndroid::SentryScopeAndroid(jobject scope)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	ScopeAndroid = Env->NewGlobalRef(scope);
}

SentryScopeAndroid::~SentryScopeAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(ScopeAndroid);
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
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "setLevel", "(Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

void SentryScopeAndroid::Clear()
{
	SentryMethodCallAndroid::CallVoidMethod(ScopeAndroid, "clear", "()V");
}
