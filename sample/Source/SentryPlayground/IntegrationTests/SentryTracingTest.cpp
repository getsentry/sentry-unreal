// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTracingTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentryLibrary.h"
#include "SentrySpan.h"
#include "SentrySubsystem.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"
#include "SentryTransactionOptions.h"
#include "SentryVariant.h"

#include "HAL/PlatformProcess.h"
#include "Misc/EngineVersionComparison.h"

void FSentryTracingTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	// Start transaction via context and options to exercise the custom sampler path
	USentryTransactionContext* TransactionContext =
		USentryLibrary::CreateSentryTransactionContext(TEXT("integration.tracing.test"), TEXT("e2e.test"));

	FSentryTransactionOptions TransactionOptions;
	TransactionOptions.CustomSamplingContext.Add(TEXT("test_key"), FSentryVariant(TEXT("test_value")));

	USentryTransaction* Transaction = Subsystem->StartTransactionWithContextAndOptions(
		TransactionContext, TransactionOptions);

	// Tags
	Transaction->SetTag(TEXT("test.type"), TEXT("tracing"));
	Transaction->SetTag(TEXT("test.suite"), TEXT("integration"));
	Transaction->SetTag(TEXT("tracing.to_be_removed"), TEXT("original_value"));
	Transaction->RemoveTag(TEXT("tracing.to_be_removed"));

	// Data
	TMap<FString, FSentryVariant> TransactionData;
	TransactionData.Add(TEXT("data_key"), FSentryVariant(TEXT("data_value")));
	Transaction->SetData(TEXT("test_data"), TransactionData);

	TMap<FString, FSentryVariant> DataToRemove;
	DataToRemove.Add(TEXT("key"), FSentryVariant(TEXT("value")));
	Transaction->SetData(TEXT("data_to_be_removed"), DataToRemove);
	Transaction->RemoveData(TEXT("data_to_be_removed"));

	// Child span
	USentrySpan* ChildSpan = Transaction->StartChildSpan(
		TEXT("e2e.child"), TEXT("Child span description"));
	ChildSpan->SetTag(TEXT("span.level"), TEXT("child"));

	TMap<FString, FSentryVariant> SpanData;
	SpanData.Add(TEXT("span_key"), FSentryVariant(TEXT("span_value")));
	ChildSpan->SetData(TEXT("span_data"), SpanData);

	// Grandchild span
	USentrySpan* GrandchildSpan = ChildSpan->StartChild(
		TEXT("e2e.grandchild"), TEXT("Grandchild span description"));
	GrandchildSpan->SetTag(TEXT("span.level"), TEXT("grandchild"));
	GrandchildSpan->SetTag(TEXT("tracing.to_be_removed"), TEXT("original_value"));
	GrandchildSpan->RemoveTag(TEXT("tracing.to_be_removed"));

	GrandchildSpan->Finish();
	ChildSpan->Finish();

	FString TraceKey;
	FString TraceValue;
	Transaction->GetTrace(TraceKey, TraceValue);

	// Parse trace ID from sentry-trace header format: <trace-id>-<span-id>-<sampled>
	FString TraceId;
	FString Remainder;
	TraceValue.Split(TEXT("-"), &TraceId, &Remainder);

	Transaction->Finish();

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("TRACE_CAPTURED: %s\n"), *TraceId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("TRACE_CAPTURED: %s\n"), *TraceId);
#endif

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(!TraceId.IsEmpty());
}
