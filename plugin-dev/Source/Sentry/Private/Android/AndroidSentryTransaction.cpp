// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryTransaction.h"
#include "AndroidSentrySpan.h"

#include "SentryDefines.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryTransaction::FAndroidSentryTransaction(jobject transaction)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Transaction, transaction)
{
	SetupClassMethods();
}

void FAndroidSentryTransaction::SetupClassMethods()
{
	StartChildMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	ToSentryTraceMethod = GetMethod("toSentryTrace", "()Lio/sentry/SentryTraceHeader;");
}

TSharedPtr<ISentrySpan> FAndroidSentryTransaction::StartChildSpan(const FString& operation, const FString& desctiption)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return MakeShareable(new FAndroidSentrySpan(*span));
}

TSharedPtr<ISentrySpan> FAndroidSentryTransaction::StartChildSpanWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Android."));
	return StartChildSpan(operation, desctiption);
}

void FAndroidSentryTransaction::Finish()
{
	CallMethod<void>(FinishMethod);
}

void FAndroidSentryTransaction::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing transaction with explicit timestamp not supported on Android."));
	Finish();
}

bool FAndroidSentryTransaction::IsFinished() const
{
	return CallMethod<bool>(IsFinishedMethod);
}

void FAndroidSentryTransaction::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

void FAndroidSentryTransaction::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), *GetJString(value));
}

void FAndroidSentryTransaction::RemoveTag(const FString& key)
{
	SetTag(key, TEXT(""));
}

void FAndroidSentryTransaction::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), FAndroidSentryConverters::VariantMapToNative(values)->GetJObject());
}

void FAndroidSentryTransaction::RemoveData(const FString& key)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), nullptr);
}

void FAndroidSentryTransaction::GetTrace(FString& name, FString& value)
{
	FSentryJavaObjectWrapper NativeTraceHeader(SentryJavaClasses::SentryTraceHeader, *CallObjectMethod<jobject>(ToSentryTraceMethod));
	FSentryJavaMethod GetValueMethod = NativeTraceHeader.GetMethod("getValue", "()Ljava/lang/String;");

	name = TEXT("sentry-trace");
	value = NativeTraceHeader.CallMethod<FString>(GetValueMethod);
}
