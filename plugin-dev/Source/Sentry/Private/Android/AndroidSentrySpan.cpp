// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentrySpan.h"

#include "SentryDefines.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentrySpan::FAndroidSentrySpan(jobject span)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Span, span)
{
	SetupClassMethods();
}

void FAndroidSentrySpan::SetupClassMethods()
{
	StartChildMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ISpan;");
	StartChildWithTimestampMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;Lio/sentry/SentryDate;)Lio/sentry/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
	FinishWithTimestampMethod = GetMethod("finish", "(Lio/sentry/SpanStatus;Lio/sentry/SentryDate;)V");
	GetStatusMethod = GetMethod("getStatus", "()Lio/sentry/SpanStatus;");
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	ToSentryTraceMethod = GetMethod("toSentryTrace", "()Lio/sentry/SentryTraceHeader;");
}

TSharedPtr<ISentrySpan> FAndroidSentrySpan::StartChild(const FString& operation, const FString& desctiption, bool bindToScope)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return MakeShareable(new FAndroidSentrySpan(*span));
}

TSharedPtr<ISentrySpan> FAndroidSentrySpan::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp, bool bindToScope)
{
	auto span = CallObjectMethod<jobject>(StartChildWithTimestampMethod, *GetJString(operation), *GetJString(desctiption), FAndroidSentryConverters::TimestampToNative(timestamp)->GetJObject());
	return MakeShareable(new FAndroidSentrySpan(*span));
}

void FAndroidSentrySpan::Finish()
{
	CallMethod<void>(FinishMethod);
}

void FAndroidSentrySpan::FinishWithTimestamp(int64 timestamp)
{
	auto status = CallObjectMethod<jobject>(GetStatusMethod);
	CallMethod<void>(FinishWithTimestampMethod, *status, FAndroidSentryConverters::TimestampToNative(timestamp)->GetJObject());
}

bool FAndroidSentrySpan::IsFinished() const
{
	return CallMethod<bool>(IsFinishedMethod);
}

void FAndroidSentrySpan::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), *GetJString(value));
}

void FAndroidSentrySpan::RemoveTag(const FString& key)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), nullptr);
}

void FAndroidSentrySpan::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), FAndroidSentryConverters::VariantMapToNative(values)->GetJObject());
}

void FAndroidSentrySpan::RemoveData(const FString& key)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), nullptr);
}

void FAndroidSentrySpan::GetTrace(FString& name, FString& value)
{
	FSentryJavaObjectWrapper NativeTraceHeader(SentryJavaClasses::SentryTraceHeader, *CallObjectMethod<jobject>(ToSentryTraceMethod));
	FSentryJavaMethod GetValueMethod = NativeTraceHeader.GetMethod("getValue", "()Ljava/lang/String;");

	name = TEXT("sentry-trace");
	value = NativeTraceHeader.CallMethod<FString>(GetValueMethod);
}
