// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionAndroid.h"
#include "SentrySpanAndroid.h"

#include "SentryDefines.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionAndroid::SentryTransactionAndroid(jobject transaction)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Transaction, transaction)
{
	SetupClassMethods();
}

void SentryTransactionAndroid::SetupClassMethods()
{
	StartChildMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	ToSentryTraceMethod = GetMethod("toSentryTrace", "()Lio/sentry/SentryTraceHeader;");
}

TSharedPtr<ISentrySpan> SentryTransactionAndroid::StartChild(const FString& operation, const FString& desctiption)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return MakeShareable(new SentrySpanAndroid(*span));
}

TSharedPtr<ISentrySpan> SentryTransactionAndroid::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Android."));
	return StartChild(operation, desctiption);
}

void SentryTransactionAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}

void SentryTransactionAndroid::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing transaction with explicit timestamp not supported on Android."));
	Finish();
}

bool SentryTransactionAndroid::IsFinished() const
{
	return CallMethod<bool>(IsFinishedMethod);;
}

void SentryTransactionAndroid::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

void SentryTransactionAndroid::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), *GetJString(value));
}

void SentryTransactionAndroid::RemoveTag(const FString& key)
{
	SetTag(key, TEXT(""));
}

void SentryTransactionAndroid::SetData(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentryTransactionAndroid::RemoveData(const FString& key)
{
	SetData(key, TMap<FString, FString>());
}

void SentryTransactionAndroid::GetTrace(FString& name, FString& value)
{
	FSentryJavaObjectWrapper NativeTraceHeader(SentryJavaClasses::SentryTraceHeader, *CallObjectMethod<jobject>(ToSentryTraceMethod));
	FSentryJavaMethod GetValueMethod = NativeTraceHeader.GetMethod("getValue", "()Ljava/lang/String;");

	name = TEXT("sentry-trace");
	value = NativeTraceHeader.CallMethod<FString>(GetValueMethod);
}
