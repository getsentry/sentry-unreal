// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

void CopySpanTracingHeader(const char *key, const char *value, void *userdata)
{
	sentry_value_t *header = static_cast<sentry_value_t*>(userdata);
	sentry_value_set_by_key(*header, key, sentry_value_new_string(value));
}

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

TSharedPtr<ISentrySpan> SentrySpanDesktop::StartChild(const FString& operation, const FString& desctiption)
{
	sentry_span_t* nativeSpan = sentry_span_start_child(SpanDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption));
	return MakeShareable(new SentrySpanDesktop(nativeSpan));
}

TSharedPtr<ISentrySpan> SentrySpanDesktop::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	sentry_span_t* nativeSpan = sentry_span_start_child_ts(SpanDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption), timestamp);
	return MakeShareable(new SentrySpanDesktop(nativeSpan));
}

void SentrySpanDesktop::Finish()
{
	sentry_span_finish(SpanDesktop);

	isFinished = true;
}

void SentrySpanDesktop::FinishWithTimestamp(int64 timestamp)
{
	sentry_span_finish_ts(SpanDesktop, timestamp);

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

void SentrySpanDesktop::GetTrace(FString& name, FString& value)
{
	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_span_iter_headers(SpanDesktop, CopySpanTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace")));

	sentry_value_decref(tracingHeader);
}

#endif
