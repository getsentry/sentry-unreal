// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanAndroid.h"

#include "SentryDefines.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentrySpanAndroid::SentrySpanAndroid(jobject span)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Span, span)
{
	SetupClassMethods();
}

void SentrySpanAndroid::SetupClassMethods()
{
	StartChildMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	ToSentryTraceMethod = GetMethod("toSentryTrace", "()Lio/sentry/SentryTraceHeader;");
}

TSharedPtr<ISentrySpan> SentrySpanAndroid::StartChild(const FString& operation, const FString& desctiption)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return MakeShareable(new SentrySpanAndroid(*span));
}

TSharedPtr<ISentrySpan> SentrySpanAndroid::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Android."));
	return StartChild(operation, desctiption);
}

void SentrySpanAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}

void SentrySpanAndroid::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing span with explicit timestamp not supported on Android."));
	Finish();
}

bool SentrySpanAndroid::IsFinished() const
{
	return CallMethod<bool>(IsFinishedMethod);;
}

void SentrySpanAndroid::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), *GetJString(value));
}

void SentrySpanAndroid::RemoveTag(const FString& key)
{
	SetTag(key, TEXT(""));
}

void SentrySpanAndroid::SetData(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentrySpanAndroid::RemoveData(const FString& key)
{
	SetData(key, TMap<FString, FString>());
}

void SentrySpanAndroid::GetTrace(FString& name, FString& value)
{
	FSentryJavaObjectWrapper NativeTraceHeader(SentryJavaClasses::SentryTraceHeader, *CallObjectMethod<jobject>(ToSentryTraceMethod));
	FSentryJavaMethod GetValueMethod = NativeTraceHeader.GetMethod("getValue", "()Ljava/lang/String;");

	name = TEXT("sentry-trace");
	value = NativeTraceHeader.CallMethod<FString>(GetValueMethod);
}
