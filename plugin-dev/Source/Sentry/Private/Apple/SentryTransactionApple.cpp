// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionApple.h"
#include "SentrySpanApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

#include "SentryDefines.h"

SentryTransactionApple::SentryTransactionApple(id<SentrySpan> transaction)
{
	TransactionApple = transaction;
	[TransactionApple retain];
}

SentryTransactionApple::~SentryTransactionApple()
{
	[TransactionApple release];
}

id<SentrySpan> SentryTransactionApple::GetNativeObject()
{
	return TransactionApple;
}

TSharedPtr<ISentrySpan> SentryTransactionApple::StartChild(const FString& operation, const FString& desctiption)
{
	id<SentrySpan> span = [TransactionApple startChildWithOperation:operation.GetNSString() description:desctiption.GetNSString()];
	return MakeShareable(new SentrySpanApple(span));
}

TSharedPtr<ISentrySpan> SentryTransactionApple::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Mac/iOS."));
	return StartChild(operation, desctiption);
}

void SentryTransactionApple::Finish()
{
	[TransactionApple finish];
}

void SentryTransactionApple::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing transaction with explicit timestamp not supported on Mac/iOS."));
	Finish();
}

bool SentryTransactionApple::IsFinished() const
{
	return TransactionApple.isFinished;
}

void SentryTransactionApple::SetName(const FString& name)
{
	// no corresponding implementation in sentry-cocoa
	UE_LOG(LogSentrySdk, Warning, TEXT("The Cocoa SDK doesn't currently support SetName function"));
}

void SentryTransactionApple::SetTag(const FString& key, const FString& value)
{
	[TransactionApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void SentryTransactionApple::RemoveTag(const FString& key)
{
	[TransactionApple removeTagForKey:key.GetNSString()];
}

void SentryTransactionApple::SetData(const FString& key, const TMap<FString, FString>& values)
{
	[TransactionApple setDataValue:SentryConvertorsApple::StringMapToNative(values) forKey:key.GetNSString()];
}

void SentryTransactionApple::RemoveData(const FString& key)
{
	[TransactionApple removeDataForKey:key.GetNSString()];
}

void SentryTransactionApple::GetTrace(FString& name, FString& value)
{
	SentryTraceHeader* traceHeader = [TransactionApple toTraceHeader];

	name = TEXT("sentry-trace");
	value = FString([traceHeader value]);
}
