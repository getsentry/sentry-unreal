// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySpan.h"

#include "SentryDefines.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

FAppleSentrySpan::FAppleSentrySpan(id<SentrySpan> span)
{
	SpanApple = span;
	[SpanApple retain];
}

FAppleSentrySpan::~FAppleSentrySpan()
{
	[SpanApple release];
}

id<SentrySpan> FAppleSentrySpan::GetNativeObject()
{
	return SpanApple;
}

TSharedPtr<ISentrySpan> FAppleSentrySpan::StartChild(const FString& operation, const FString& desctiption)
{
	id<SentrySpan> span = [SpanApple startChildWithOperation:operation.GetNSString() description:desctiption.GetNSString()];
	return MakeShareable(new FAppleSentrySpan(span));
}

TSharedPtr<ISentrySpan> FAppleSentrySpan::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Starting child span with explicit timestamp not supported on Mac/iOS."));
	return StartChild(operation, desctiption);
}

void FAppleSentrySpan::Finish()
{
	[SpanApple finish];
}

void FAppleSentrySpan::FinishWithTimestamp(int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Finishing span with explicit timestamp not supported on Mac/iOS."));
	Finish();
}

bool FAppleSentrySpan::IsFinished() const
{
	return SpanApple.isFinished;
}

void FAppleSentrySpan::SetTag(const FString& key, const FString& value)
{
	[SpanApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void FAppleSentrySpan::RemoveTag(const FString& key)
{
	[SpanApple removeTagForKey:key.GetNSString()];
}

void FAppleSentrySpan::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	[SpanApple setDataValue:FAppleSentryConverters::VariantMapToNative(values) forKey:key.GetNSString()];
}

void FAppleSentrySpan::RemoveData(const FString& key)
{
	[SpanApple removeDataForKey:key.GetNSString()];
}

void FAppleSentrySpan::GetTrace(FString& name, FString& value)
{
	SentryTraceHeader* traceHeader = [SpanApple toTraceHeader];

	name = TEXT("sentry-trace");
	value = FString([traceHeader value]);
}
