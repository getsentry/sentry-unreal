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
{
}

sentry_span_t* FGenericPlatformSentrySpan::GetNativeObject()
{
	FScopeLock Lock(&CriticalSection);

	return Span;
}

TSharedPtr<ISentrySpan> FGenericPlatformSentrySpan::StartChild(const FString& operation, const FString& description, bool bindToScope)
{
	FScopeLock Lock(&CriticalSection);

	if (!Span)
	{
		return nullptr;
	}

	if (sentry_span_t* nativeSpan = sentry_span_start_child(Span, TCHAR_TO_UTF8(*operation), TCHAR_TO_UTF8(*description)))
	{
		if (bindToScope)
		{
			sentry_set_span(nativeSpan);
		}

		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentrySpan> FGenericPlatformSentrySpan::StartChildWithTimestamp(const FString& operation, const FString& description, int64 timestamp, bool bindToScope)
{
	FScopeLock Lock(&CriticalSection);

	if (!Span)
	{
		return nullptr;
	}

	if (sentry_span_t* nativeSpan = sentry_span_start_child_ts(Span, TCHAR_TO_UTF8(*operation), TCHAR_TO_UTF8(*description), timestamp))
	{
		if (bindToScope)
		{
			sentry_set_span(nativeSpan);
		}

		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

void FGenericPlatformSentrySpan::Finish()
{
	FScopeLock Lock(&CriticalSection);

	if (!Span)
	{
		return;
	}

	// sentry-native takes ownership of the span on finish
	sentry_span_finish(Span);
	Span = nullptr;
}

void FGenericPlatformSentrySpan::FinishWithTimestamp(int64 timestamp)
{
	FScopeLock Lock(&CriticalSection);

	if (!Span)
	{
		return;
	}

	sentry_span_finish_ts(Span, timestamp);
	Span = nullptr;
}

bool FGenericPlatformSentrySpan::IsFinished() const
{
	FScopeLock Lock(&CriticalSection);

	return Span == nullptr;
}

void FGenericPlatformSentrySpan::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	if (Span)
	{
		sentry_span_set_tag(Span, TCHAR_TO_UTF8(*key), TCHAR_TO_UTF8(*value));
	}
}

void FGenericPlatformSentrySpan::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	if (Span)
	{
		sentry_span_remove_tag(Span, TCHAR_TO_UTF8(*key));
	}
}

void FGenericPlatformSentrySpan::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FScopeLock Lock(&CriticalSection);

	if (Span)
	{
		sentry_span_set_data(Span, TCHAR_TO_UTF8(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));
	}
}

void FGenericPlatformSentrySpan::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	if (Span)
	{
		sentry_span_remove_data(Span, TCHAR_TO_UTF8(*key));
	}
}

void FGenericPlatformSentrySpan::GetTrace(FString& name, FString& value)
{
	FScopeLock Lock(&CriticalSection);

	if (!Span)
	{
		return;
	}

	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_span_iter_headers(Span, CopySpanTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(UTF8_TO_TCHAR(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace"))));

	sentry_value_decref(tracingHeader);
}

#endif
