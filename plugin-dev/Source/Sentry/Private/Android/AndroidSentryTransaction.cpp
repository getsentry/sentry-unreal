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
	StartChildWithTimestampMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;Lio/sentry/SentryDate;)Lio/sentry/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
	FinishWithTimestampMethod = GetMethod("finish", "(Lio/sentry/SpanStatus;Lio/sentry/SentryDate;)V");
	GetStatusMethod = GetMethod("getStatus", "()Lio/sentry/SpanStatus;");
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
	ToSentryTraceMethod = GetMethod("toSentryTrace", "()Lio/sentry/SentryTraceHeader;");
	ToBaggageHeaderMethod = GetMethod("toBaggageHeader", "(Ljava/util/List;)Lio/sentry/BaggageHeader;");
	IsNoOpMethod = GetMethod("isNoOp", "()Z");
}

TSharedPtr<ISentrySpan> FAndroidSentryTransaction::StartChildSpan(const FString& operation, const FString& desctiption, bool bindToScope)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return MakeShareable(new FAndroidSentrySpan(*span));
}

TSharedPtr<ISentrySpan> FAndroidSentryTransaction::StartChildSpanWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp, bool bindToScope)
{
	auto span = CallObjectMethod<jobject>(StartChildWithTimestampMethod, *GetJString(operation), *GetJString(desctiption), FAndroidSentryConverters::TimestampToNative(timestamp)->GetJObject());
	return MakeShareable(new FAndroidSentrySpan(*span));
}

void FAndroidSentryTransaction::Finish()
{
	CallMethod<void>(FinishMethod);
}

void FAndroidSentryTransaction::FinishWithTimestamp(int64 timestamp)
{
	auto status = CallObjectMethod<jobject>(GetStatusMethod);
	CallMethod<void>(FinishWithTimestampMethod, *status, FAndroidSentryConverters::TimestampToNative(timestamp)->GetJObject());
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
	CallMethod<void>(SetTagMethod, *GetJString(key), nullptr);
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

TMap<FString, FString> FAndroidSentryTransaction::GetTraceHeaders()
{
	TMap<FString, FString> headers;

	if (CallMethod<bool>(IsNoOpMethod))
	{
		return headers;
	}

	FSentryJavaObjectWrapper NativeTraceHeader(SentryJavaClasses::SentryTraceHeader, *CallObjectMethod<jobject>(ToSentryTraceMethod));
	FSentryJavaMethod GetTraceValueMethod = NativeTraceHeader.GetMethod("getValue", "()Ljava/lang/String;");

	headers.Add(TEXT("sentry-trace"), NativeTraceHeader.CallMethod<FString>(GetTraceValueMethod));

	auto baggage = CallObjectMethod<jobject>(ToBaggageHeaderMethod, nullptr);
	if (baggage)
	{
		FSentryJavaObjectWrapper NativeBaggageHeader(SentryJavaClasses::BaggageHeader, *baggage);
		FSentryJavaMethod GetBaggageValueMethod = NativeBaggageHeader.GetMethod("getValue", "()Ljava/lang/String;");

		headers.Add(TEXT("baggage"), NativeBaggageHeader.CallMethod<FString>(GetBaggageValueMethod));
	}

	return headers;
}
