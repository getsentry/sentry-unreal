// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "Android/AndroidSentryBreadcrumb.h"
#include "Android/AndroidSentryEvent.h"
#include "Android/AndroidSentryHint.h"
#include "Android/AndroidSentrySamplingContext.h"
#include "Android/AndroidSentryScope.h"
#include "Android/AndroidSentrySubsystem.h"
#include "Android/Callbacks/AndroidSentryScopeCallback.h"
#include "Android/Infrastructure/AndroidSentryConverters.h"
#include "Android/Infrastructure/AndroidSentryJavaClasses.h"

#include "Android/AndroidJNI.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentrySamplingContext.h"
#include "SentryTraceSampler.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"
#include "Utils/SentryFileUtils.h"

JNI_METHOD void Java_io_sentry_unreal_SentryBridgeJava_onConfigureScope(JNIEnv* env, jclass clazz, jlong callbackId, jobject scope)
{
	FGCScopeGuard GCScopeGuard;

	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `onConfigureScope` handler is not allowed when post-loading."));
		return;
	}

	FSentryScopeDelegate* callback = AndroidSentryScopeCallback::GetDelegateById(callbackId);

	if (callback != nullptr)
	{
		callback->Execute(MakeShareable(new FAndroidSentryScope(scope)));
		AndroidSentryScopeCallback::RemoveDelegate(callbackId);
	}
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeSend(JNIEnv* env, jclass clazz, jlong objAddr, jobject event, jobject hint)
{
	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during object post-loading."));
		return event;
	}

	if (IsGarbageCollecting())
	{
		// If event is captured during garbage collection we can't instantiate UObjects safely or obtain a GC lock
		// since it will cause a deadlock (see https://github.com/getsentry/sentry-unreal/issues/850).
		// In this case event will be reported without calling a `beforeSend` handler.
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during garbage collection."));
		return event;
	}

	USentryBeforeSendHandler* handler = reinterpret_cast<USentryBeforeSendHandler*>(objAddr);

	USentryEvent* EventToProcess = USentryEvent::Create(MakeShareable(new FAndroidSentryEvent(event)));
	USentryHint* HintToProcess = USentryHint::Create(MakeShareable(new FAndroidSentryHint(hint)));

	USentryEvent* ProcessedEvent = handler->HandleBeforeSend(EventToProcess, HintToProcess);

	return ProcessedEvent ? event : nullptr;
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeBreadcrumb(JNIEnv* env, jclass clazz, jlong objAddr, jobject breadcrumb, jobject hint)
{
	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		// Don't print to logs within `onBeforeBreadcrumb` handler as this can lead to creating new breadcrumb
		return breadcrumb;
	}

	if (IsGarbageCollecting())
	{
		// If breadcrumb is added during garbage collection we can't instantiate UObjects safely or obtain a GC lock
		// since there is no guarantee it will be ever freed.
		// In this case breadcrumb will be added without calling a `beforeBreadcrumb` handler.
		return breadcrumb;
	}

	USentryBeforeBreadcrumbHandler* handler = reinterpret_cast<USentryBeforeBreadcrumbHandler*>(objAddr);

	USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FAndroidSentryBreadcrumb(breadcrumb)));
	USentryHint* HintToProcess = USentryHint::Create(MakeShareable(new FAndroidSentryHint(hint)));

	USentryBreadcrumb* ProcessedBreadcrumb = handler->HandleBeforeBreadcrumb(BreadcrumbToProcess, HintToProcess);

	return ProcessedBreadcrumb ? breadcrumb : nullptr;
}

JNI_METHOD jfloat Java_io_sentry_unreal_SentryBridgeJava_onTracesSampler(JNIEnv* env, jclass clazz, jlong objAddr, jobject samplingContext)
{
	FGCScopeGuard GCScopeGuard;

	USentryTraceSampler* sampler = reinterpret_cast<USentryTraceSampler*>(objAddr);

	USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new FAndroidSentrySamplingContext(samplingContext)));

	float samplingValue;
	if (sampler->Sample(Context, samplingValue))
	{
		return (jfloat)samplingValue;
	}

	// to avoid instantiating `java.lang.Double` object within this JNI callback a negative value is returned instead
	// which should be interpreted as `null` in Java code to fallback to fixed sample rate value
	return -1.0f;
}

JNI_METHOD jstring Java_io_sentry_unreal_SentryBridgeJava_getLogFilePath(JNIEnv* env, jclass clazz, jboolean isCrash)
{
	const FString LogFilePath = isCrash ? SentryFileUtils::GetGameLogBackupPath() : SentryFileUtils::GetGameLogPath();

	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.FileExists(*LogFilePath))
	{
		return *FSentryJavaObjectWrapper::GetJString(FString(""));
	}

	return *FSentryJavaObjectWrapper::GetJString(LogFilePath);
}
