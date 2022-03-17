// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "Android/Callbacks/SentryScopeCallbackAndroid.h"
#include "Android/Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidJNI.h"

JNI_METHOD void Java_com_sentry_unreal_SentryJava_onConfigureScope(JNIEnv* env, jclass clazz, jlong objAddr, jobject scope)
{
	USentryScopeCallbackAndroid* callback = reinterpret_cast<USentryScopeCallbackAndroid*>(objAddr);

	if (IsValid(callback))
	{
		callback->ExecuteDelegate(SentryConvertorsAndroid::SentryScopeToUnreal(scope));
	}
}