// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySpan.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

void CopySpanTracingHeader(const char* key, const char* value, void* userdata)
{
	sentry_value_t* header = static_cast<sentry_value_t*>(userdata);
	sentry_value_set_by_key(*header, key, sentry_value_new_string(value));
}

FGenericPlatformSentrySpan::FGenericPlatformSentrySpan(sentry_span_t* span)
	: Span(span)
	, isFinished(false)
{
}

sentry_span_t* FGenericPlatformSentrySpan::GetNativeObject()
{
	return Span;
}

TSharedPtr<ISentrySpan> FGenericPlatformSentrySpan::StartChild(const FString& operation, const FString& description)
{
	if (sentry_span_t* nativeSpan = sentry_span_start_child(Span, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*description)))
	{
		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentrySpan> FGenericPlatformSentrySpan::StartChildWithTimestamp(const FString& operation, const FString& description, int64 timestamp)
{
	if (sentry_span_t* nativeSpan = sentry_span_start_child_ts(Span, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*description), timestamp))
	{
		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

void FGenericPlatformSentrySpan::Finish()
{
	sentry_span_finish(Span);

	isFinished = true;
}

void FGenericPlatformSentrySpan::FinishWithTimestamp(int64 timestamp)
{
	sentry_span_finish_ts(Span, timestamp);

	isFinished = true;
}

bool FGenericPlatformSentrySpan::IsFinished() const
{
	return isFinished;
}

void FGenericPlatformSentrySpan::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_set_tag(Span, TCHAR_TO_ANSI(*key), TCHAR_TO_ANSI(*value));
}

void FGenericPlatformSentrySpan::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_remove_tag(Span, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentrySpan::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_set_data(Span, TCHAR_TO_ANSI(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));
}

void FGenericPlatformSentrySpan::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_span_remove_data(Span, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentrySpan::GetTrace(FString& name, FString& value)
{
	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_span_iter_headers(Span, CopySpanTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace")));

	sentry_value_decref(tracingHeader);
}

#endif
