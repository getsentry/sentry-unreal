// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanApple.h"

SentrySpanApple::SentrySpanApple(id<SentrySpan> span)
{
	SpanApple = span;
}

SentrySpanApple::~SentrySpanApple()
{
	// Put custom destructor logic here if needed
}

id<SentrySpan> SentrySpanApple::GetNativeObject()
{
	return SpanApple;
}

void SentrySpanApple::Finish()
{
	[SpanApple finish];
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
