// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "Android/AndroidSentryBreadcrumb.h"
#include "Android/AndroidSentryEvent.h"
#include "Android/AndroidSentryHint.h"
#include "Android/AndroidSentryLog.h"
#include "Android/AndroidSentrySamplingContext.h"
#include "Android/AndroidSentryScope.h"
#include "Android/AndroidSentrySubsystem.h"
#include "Android/Callbacks/AndroidSentryScopeCallback.h"
#include "Android/Infrastructure/AndroidSentryConverters.h"
#include "Android/Infrastructure/AndroidSentryJavaClasses.h"

#include "Android/AndroidJNI.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeLogHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryLog.h"
#include "SentrySamplingContext.h"
#include "SentryTraceSampler.h"

#include "Utils/SentryCallbackUtils.h"
#include "Utils/SentryFileUtils.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

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
	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		// Event will be sent without calling a `onBeforeSend` handler
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
	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		// Breadcrumb will be added without calling a `beforeBreadcrumb` handler
		return breadcrumb;
	}

	USentryBeforeBreadcrumbHandler* handler = reinterpret_cast<USentryBeforeBreadcrumbHandler*>(objAddr);

	USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FAndroidSentryBreadcrumb(breadcrumb)));
	USentryHint* HintToProcess = USentryHint::Create(MakeShareable(new FAndroidSentryHint(hint)));

	USentryBreadcrumb* ProcessedBreadcrumb = handler->HandleBeforeBreadcrumb(BreadcrumbToProcess, HintToProcess);

	return ProcessedBreadcrumb ? breadcrumb : nullptr;
}

JNI_METHOD jobject Java_io_sentry_unreal_SentryBridgeJava_onBeforeLog(JNIEnv* env, jclass clazz, jlong objAddr, jobject logEvent)
{
	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		// Log will be added without calling a `onBeforeLog` handler
		return logEvent;
	}

	USentryBeforeLogHandler* handler = reinterpret_cast<USentryBeforeLogHandler*>(objAddr);

	USentryLog* LogDataToProcess = USentryLog::Create(MakeShareable(new FAndroidSentryLog(logEvent)));

	USentryLog* ProcessedLogData = handler->HandleBeforeLog(LogDataToProcess);

	return ProcessedLogData ? logEvent : nullptr;
}

JNI_METHOD jfloat Java_io_sentry_unreal_SentryBridgeJava_onTracesSampler(JNIEnv* env, jclass clazz, jlong objAddr, jobject samplingContext)
{
	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		// Falling back to default sampling value without calling a custom sampling function
		return -1.0f;
	}

	USentryTraceSampler* sampler = reinterpret_cast<USentryTraceSampler*>(objAddr);

	USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new FAndroidSentrySamplingContext(samplingContext)));

	float samplingValue;
	if (sampler->Sample(Context, samplingValue))
	{
		return (jfloat)samplingValue;
	}

	// To avoid instantiating `java.lang.Double` object within this JNI callback a negative value is returned instead
	// which should be interpreted as `null` in Java code to fallback to fixed sample rate value
	return -1.0f;
}

JNI_METHOD jstring Java_io_sentry_unreal_SentryBridgeJava_getLogFilePath(JNIEnv* env, jclass clazz, jboolean isCrash)
{
	const FString LogFilePath = isCrash ? SentryFileUtils::GetGameLogBackupPath() : SentryFileUtils::GetGameLogPath();

	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.FileExists(*LogFilePath))
	{
		return env->NewStringUTF("");
	}

	return env->NewStringUTF(TCHAR_TO_UTF8(*LogFilePath));
}

JNI_METHOD jstring Java_io_sentry_unreal_SentryBridgeJava_getScreenshotFilePath(JNIEnv* env, jclass clazz)
{
	const FString ScreenshotFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SentryFileUtils::GetLatestScreenshot());

	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.FileExists(*ScreenshotFilePath))
	{
		return env->NewStringUTF("");
	}

	return env->NewStringUTF(TCHAR_TO_UTF8(*ScreenshotFilePath));
}
