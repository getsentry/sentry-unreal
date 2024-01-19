// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentrySpanDesktop::SentrySpanDesktop(sentry_span_t* span)
	: SpanDesktop(span)
	, isFinished(false)
{
}

SentrySpanDesktop::~SentrySpanDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_span_t* SentrySpanDesktop::GetNativeObject()
{
	return SpanDesktop;
}

void SentrySpanDesktop::Finish()
{
	sentry_span_finish(SpanDesktop);

	isFinished = true;
}

bool SentrySpanDesktop::IsFinished() const
{
	return isFinished;
}

void SentrySpanDesktop::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_set_tag(SpanDesktop, TCHAR_TO_ANSI(*key), TCHAR_TO_ANSI(*value));
}

void SentrySpanDesktop::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_remove_tag(SpanDesktop, TCHAR_TO_ANSI(*key));
}

void SentrySpanDesktop::SetData(const FString& key, const TMap<FString, FString>& values)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_set_data(SpanDesktop, TCHAR_TO_ANSI(*key), SentryConvertorsDesktop::StringMapToNative(values));
}

void SentrySpanDesktop::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_remove_data(SpanDesktop, TCHAR_TO_ANSI(*key));
}

#endif
