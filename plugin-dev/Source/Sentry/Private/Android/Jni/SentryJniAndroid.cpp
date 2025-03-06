// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "Android/Callbacks/SentryScopeCallbackAndroid.h"
#include "Android/Infrastructure/SentryConvertersAndroid.h"
#include "Android/Infrastructure/SentryJavaClasses.h"
#include "Android/AndroidSentrySubsystem.h"
#include "Android/SentryScopeAndroid.h"
#include "Android/SentryBreadcrumbAndroid.h"
#include "Android/SentryEventAndroid.h"
#include "Android/SentryHintAndroid.h"
#include "Android/SentrySamplingContextAndroid.h"

#include "Android/AndroidJNI.h"

#include "SentryDefines.h"
#include "SentryBreadcrumb.h"
#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryTraceSampler.h"
#include "SentrySamplingContext.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

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

	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed when post-loading."));
		return event;
	}

	USentryBeforeSendHandler* handler = reinterpret_cast<USentryBeforeSendHandler*>(objAddr);

	USentryEvent* EventToProcess = USentryEvent::Create(MakeShareable(new SentryEventAndroid(event)));
	USentryHint* HintToProcess = USentryHint::Create(MakeShareable(new SentryHintAndroid(hint)));

	USentryEvent* ProcessedEvent = handler->HandleBeforeSend(EventToProcess, HintToProcess);

	return ProcessedEvent ? event : nullptr;
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeBreadcrumb(JNIEnv* env, jclass clazz, jlong objAddr, jobject breadcrumb, jobject hint)
{
	FTaskTagScope Scope(ETaskTag::EGameThread);

	FGCScopeGuard GCScopeGuard;

	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `onBeforeBreadcrumb` handler is not allowed when post-loading."));
		return breadcrumb;
	}

	USentryBeforeBreadcrumbHandler* handler = reinterpret_cast<USentryBeforeBreadcrumbHandler*>(objAddr);

	USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new SentryBreadcrumbAndroid(breadcrumb)));
	USentryHint* HintToProcess = USentryHint::Create(MakeShareable(new SentryHintAndroid(hint)));

	USentryBreadcrumb* ProcessedBreadcrumb = handler->HandleBeforeBreadcrumb(BreadcrumbToProcess, HintToProcess);

	return ProcessedBreadcrumb ? breadcrumb : nullptr;
}

JNI_METHOD jfloat Java_io_sentry_unreal_SentryBridgeJava_onTracesSampler(JNIEnv* env, jclass clazz, jlong objAddr, jobject samplingContext)
{
	FGCScopeGuard GCScopeGuard;

	USentryTraceSampler* sampler = reinterpret_cast<USentryTraceSampler*>(objAddr);

	USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new SentrySamplingContextAndroid(samplingContext)));

	float samplingValue;
	if(sampler->Sample(Context, samplingValue))
	{
		return (jfloat)samplingValue;
	}

	// to avoid instantiating `java.lang.Double` object within this JNI callback a negative value is returned instead
	// which should be interpreted as `null` in Java code to fallback to fixed sample rate value
	return -1.0f;
}