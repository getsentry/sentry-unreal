// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeAndroid.h"
#include "SentryMethodCallAndroid.h"
#include "SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryScopeAndroid::~SentryScopeAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(_scopeAndroid);
}

void SentryScopeAndroid::Init()
{
	// TODO Some default native object initialization is required
}

void SentryScopeAndroid::InitWithNativeObject(jobject scope)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	_scopeAndroid = Env->NewGlobalRef(scope);
}

jobject SentryScopeAndroid::GetNativeObject()
{
	return _scopeAndroid;
}

void SentryScopeAndroid::SetTagValue(const FString& key, const FString& value)
{
	// TODO
}

void SentryScopeAndroid::RemoveTag(const FString& key)
{
	// TODO
}

void SentryScopeAndroid::SetTags(const TMap<FString, FString>& tags)
{
	// TODO
}

void SentryScopeAndroid::SetDist(const FString& dist)
{
	// TODO
}

void SentryScopeAndroid::SetEnvironment(const FString& environment)
{
	// TODO
}

void SentryScopeAndroid::SetFingerprint(const TArray<FString>& fingerprint)
{
	// TODO
}

void SentryScopeAndroid::SetLevel(ESentryLevel level)
{
	SentryMethodCallAndroid::CallStaticVoidMethod("com/sentry/unreal/SentryJava", "setScopeLevel", "(Lio/sentry/Scope;I)V",
		_scopeAndroid, (int)level);
}

void SentryScopeAndroid::Clear()
{
	// TODO
}
