// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "Android/Callbacks/SentryScopeCallbackAndroid.h"
#include "Android/Infrastructure/SentryConvertorsAndroid.h"
#include "Android/Infrastructure/SentryJavaClasses.h"
#include "Android/SentryEventAndroid.h"
#include "Android/SentryHintAndroid.h"
#include "Android/SentrySamplingContextAndroid.h"

#include "Android/AndroidJNI.h"

#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentrySamplingContext.h"

JNI_METHOD void Java_io_sentry_unreal_SentryBridgeJava_onConfigureScope(JNIEnv* env, jclass clazz, jlong objAddr, jobject scope)
{
	USentryScopeCallbackAndroid* callback = reinterpret_cast<USentryScopeCallbackAndroid*>(objAddr);

	if (IsValid(callback))
	{
		callback->ExecuteDelegate(SentryConvertorsAndroid::SentryScopeToUnreal(scope));
	}
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeSend(JNIEnv* env, jclass clazz, jlong objAddr, jobject event, jobject hint)
{
	USentryBeforeSendHandler* handler = reinterpret_cast<USentryBeforeSendHandler*>(objAddr);

	USentryEvent* EventToProcess = NewObject<USentryEvent>();
	EventToProcess->InitWithNativeImpl(MakeShareable(new SentryEventAndroid(event)));
	USentryHint* HintToProcess = NewObject<USentryHint>();
	HintToProcess->InitWithNativeImpl(MakeShareable(new SentryHintAndroid(hint)));

	return handler->HandleBeforeSend(EventToProcess, HintToProcess) ? event : nullptr;
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onTracesSampler(JNIEnv* env, jclass clazz, jlong objAddr, jobject samplingContext)
{
	USentryTraceSampler* sampler = reinterpret_cast<USentryTraceSampler*>(objAddr);

	USentrySamplingContext* Context = NewObject<USentrySamplingContext>();
	Context->InitWithNativeImpl(MakeShareable(new SentrySamplingContextAndroid(samplingContext)));

	float samplingValue;
	if(sampler->Sample(Context, samplingValue))
	{
		TSharedPtr<FSentryJavaObjectWrapper> NativeDouble = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::Double, "(D)V", samplingValue));
		return NativeDouble->GetJObject();
	}

	return nullptr;
}