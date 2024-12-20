// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanApple.h"

#include "SentryDefines.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentrySpanApple::SentrySpanApple(id<SentrySpan> span)
{
	SpanApple = span;
	[SpanApple retain];
}

SentrySpanApple::~SentrySpanApple()
{
	[SpanApple release];
}

id<SentrySpan> SentrySpanApple::GetNativeObject()
{
	return SpanApple;
}

TSharedPtr<ISentrySpan> SentrySpanApple::StartChild(const FString& operation, const FString& desctiption)
{
	id<SentrySpan> span = [SpanApple startChildWithOperation:operation.GetNSString() description:desctiption.GetNSString()];
	return MakeShareable(new SentrySpanApple(span));
}

TSharedPtr<ISentrySpan> SentrySpanApple::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Mac/iOS."));
	return StartChild(operation, desctiption);
}

void SentrySpanApple::Finish()
{
	[SpanApple finish];
}

void SentrySpanApple::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing span with explicit timestamp not supported on Mac/iOS."));
	Finish();
}

bool SentrySpanApple::IsFinished() const
{
	return SpanApple.isFinished;
}

void SentrySpanApple::SetTag(const FString& key, const FString& value)
{
	[SpanApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void SentrySpanApple::RemoveTag(const FString& key)
{
	[SpanApple removeTagForKey:key.GetNSString()];
}

void SentrySpanApple::SetData(const FString& key, const TMap<FString, FString>& values)
{
	[SpanApple setDataValue:SentryConvertorsApple::StringMapToNative(values) forKey:key.GetNSString()];
}

void SentrySpanApple::RemoveData(const FString& key)
{
	[SpanApple removeDataForKey:key.GetNSString()];
}

void SentrySpanApple::GetTrace(FString& name, FString& value)
{
	SentryTraceHeader* traceHeader = [SpanApple toTraceHeader];

	name = TEXT("sentry-trace");
	value = FString([traceHeader value]);
}
