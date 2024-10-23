// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "Android/Callbacks/SentryScopeCallbackAndroid.h"
#include "Android/Infrastructure/SentryConvertorsAndroid.h"
#include "Android/Infrastructure/SentryJavaClasses.h"
#include "Android/SentrySubsystemAndroid.h"
#include "Android/SentryScopeAndroid.h"
#include "Android/SentryEventAndroid.h"
#include "Android/SentryHintAndroid.h"
#include "Android/SentrySamplingContextAndroid.h"

#include "Android/AndroidJNI.h"
#include "UObject/GarbageCollection.h"

#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentrySamplingContext.h"

JNI_METHOD void Java_io_sentry_unreal_SentryBridgeJava_onConfigureScope(JNIEnv* env, jclass clazz, jlong callbackId, jobject scope)
{
	FSentryScopeDelegate* callback = SentryScopeCallbackAndroid::GetDelegateById(callbackId);

	if (callback != nullptr)
	{
		callback->Execute(MakeShareable(new SentryScopeAndroid(scope)));
		SentryScopeCallbackAndroid::RemoveDelegate(callbackId);
	}
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeSend(JNIEnv* env, jclass clazz, jlong objAddr, jobject event, jobject hint)
{
	FGCScopeGuard GCScopeGuard;

	USentryBeforeSendHandler* handler = reinterpret_cast<USentryBeforeSendHandler*>(objAddr);

	USentryEvent* EventToProcess = NewObject<USentryEvent>();
	EventToProcess->InitWithNativeImpl(MakeShareable(new SentryEventAndroid(event)));
	USentryHint* HintToProcess = NewObject<USentryHint>();
	HintToProcess->InitWithNativeImpl(MakeShareable(new SentryHintAndroid(hint)));

	return handler->HandleBeforeSend(EventToProcess, HintToProcess) ? event : nullptr;
}

JNI_METHOD jfloat Java_io_sentry_unreal_SentryBridgeJava_onTracesSampler(JNIEnv* env, jclass clazz, jlong objAddr, jobject samplingContext)
{
	FGCScopeGuard GCScopeGuard;

	USentryTraceSampler* sampler = reinterpret_cast<USentryTraceSampler*>(objAddr);

	USentrySamplingContext* Context = NewObject<USentrySamplingContext>();
	Context->InitWithNativeImpl(MakeShareable(new SentrySamplingContextAndroid(samplingContext)));

	float samplingValue;
	if(sampler->Sample(Context, samplingValue))
	{
		return (jfloat)samplingValue;
	}

	// to avoid instantiating `java.lang.Double` object within this JNI callback a negative value is returned instead
	// which should be interpreted as `null` in Java code to fallback to fixed sample rate value
	return -1.0f;
}