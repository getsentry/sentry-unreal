// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTracingTimestampsTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentryLibrary.h"
#include "SentrySpan.h"
#include "SentrySubsystem.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"

#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersionComparison.h"

void FSentryTracingTimestampsTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	const int64 Now = (FDateTime::UtcNow() - FDateTime(1970, 1, 1)).GetTicks() / ETimespan::TicksPerMicrosecond;
	const int64 Second = 1000000;

	USentryTransactionContext* TransactionContext =
		USentryLibrary::CreateSentryTransactionContext(TEXT("integration.tracing.test"), TEXT("e2e.timestamps"));

	USentryTransaction* Transaction = Subsystem->StartTransactionWithContextAndTimestamp(TransactionContext, Now - 10 * Second);

	USentrySpan* ChildSpan = Transaction->StartChildSpanWithTimestamp(
		TEXT("e2e.timestamps.child"), TEXT("Timestamped child span"), Now - 8 * Second);
	ChildSpan->FinishWithTimestamp(Now - 5 * Second);

	FString TraceKey;
	FString TraceValue;
	Transaction->GetTrace(TraceKey, TraceValue);

	FString TraceId;
	FString Remainder;
	TraceValue.Split(TEXT("-"), &TraceId, &Remainder);

	Transaction->FinishWithTimestamp(Now - 2 * Second);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("TRACE_CAPTURED: %s\n"), *TraceId);
	UE_LOG(LogSentrySample, Log, TEXT("TRACE_BASE_TIMESTAMP: %lld\n"), Now);
#else
	UE_LOG(LogSentrySample, Display, TEXT("TRACE_CAPTURED: %s\n"), *TraceId);
	UE_LOG(LogSentrySample, Display, TEXT("TRACE_BASE_TIMESTAMP: %lld\n"), Now);
#endif

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(!TraceId.IsEmpty());
}
