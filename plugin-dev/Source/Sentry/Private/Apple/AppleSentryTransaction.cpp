// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryTransaction.h"
#include "AppleSentrySpan.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

#include "SentryDefines.h"

FAppleSentryTransaction::FAppleSentryTransaction(id<SentrySpan> transaction)
{
	TransactionApple = transaction;
	[TransactionApple retain];
}

FAppleSentryTransaction::~FAppleSentryTransaction()
{
	[TransactionApple release];
}

id<SentrySpan> FAppleSentryTransaction::GetNativeObject()
{
	return TransactionApple;
}

TSharedPtr<ISentrySpan> FAppleSentryTransaction::StartChildSpan(const FString& operation, const FString& desctiption)
{
	id<SentrySpan> span = [TransactionApple startChildWithOperation:operation.GetNSString() description:desctiption.GetNSString()];
	return MakeShareable(new FAppleSentrySpan(span));
}

TSharedPtr<ISentrySpan> FAppleSentryTransaction::StartChildSpanWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Mac/iOS."));
	return StartChildSpan(operation, desctiption);
}

void FAppleSentryTransaction::Finish()
{
	[TransactionApple finish];
}

void FAppleSentryTransaction::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing transaction with explicit timestamp not supported on Mac/iOS."));
	Finish();
}

bool FAppleSentryTransaction::IsFinished() const
{
	return TransactionApple.isFinished;
}

void FAppleSentryTransaction::SetName(const FString& name)
{
	// no corresponding implementation in sentry-cocoa
	UE_LOG(LogSentrySdk, Warning, TEXT("The Cocoa SDK doesn't currently support SetName function"));
}

void FAppleSentryTransaction::SetTag(const FString& key, const FString& value)
{
	[TransactionApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void FAppleSentryTransaction::RemoveTag(const FString& key)
{
	[TransactionApple removeTagForKey:key.GetNSString()];
}

void FAppleSentryTransaction::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	[TransactionApple setDataValue:FAppleSentryConverters::VariantMapToNative(values) forKey:key.GetNSString()];
}

void FAppleSentryTransaction::RemoveData(const FString& key)
{
	[TransactionApple removeDataForKey:key.GetNSString()];
}

void FAppleSentryTransaction::GetTrace(FString& name, FString& value)
{
	SentryTraceHeader* traceHeader = [TransactionApple toTraceHeader];

	name = TEXT("sentry-trace");
	value = FString([traceHeader value]);
}
