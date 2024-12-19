// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionDesktop.h"
#include "SentrySpanDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

void CopyTransactionTracingHeader(const char *key, const char *value, void *userdata)
{
	sentry_value_t *header = static_cast<sentry_value_t*>(userdata);
	sentry_value_set_by_key(*header, key, sentry_value_new_string(value));
}

SentryTransactionDesktop::SentryTransactionDesktop(sentry_transaction_t* transaction)
	: TransactionDesktop(transaction)
	, isFinished(false)
{
}

SentryTransactionDesktop::~SentryTransactionDesktop()
{
}

sentry_transaction_t* SentryTransactionDesktop::GetNativeObject()
{
	return TransactionDesktop;
}

TSharedPtr<ISentrySpan> SentryTransactionDesktop::StartChild(const FString& operation, const FString& desctiption)
{
	sentry_span_t* nativeSpan = sentry_transaction_start_child(TransactionDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption));
	return MakeShareable(new SentrySpanDesktop(nativeSpan));
}

TSharedPtr<ISentrySpan> SentryTransactionDesktop::StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp)
{
	sentry_span_t* nativeSpan = sentry_transaction_start_child_ts(TransactionDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption), timestamp);
	return MakeShareable(new SentrySpanDesktop(nativeSpan));
}

void SentryTransactionDesktop::Finish()
{
	sentry_transaction_finish(TransactionDesktop);

	isFinished = true;
}

void SentryTransactionDesktop::FinishWithTimestamp(int64 timestamp)
{
	sentry_transaction_finish_ts(TransactionDesktop, timestamp);

	isFinished = true;
}

bool SentryTransactionDesktop::IsFinished() const
{
	return isFinished;
}

void SentryTransactionDesktop::SetName(const FString& name)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_name(TransactionDesktop, TCHAR_TO_ANSI(*name));
}

void SentryTransactionDesktop::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_tag(TransactionDesktop, TCHAR_TO_ANSI(*key), TCHAR_TO_ANSI(*value));
}

void SentryTransactionDesktop::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_remove_tag(TransactionDesktop, TCHAR_TO_ANSI(*key));
}

void SentryTransactionDesktop::SetData(const FString& key, const TMap<FString, FString>& values)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_data(TransactionDesktop, TCHAR_TO_ANSI(*key), SentryConvertorsDesktop::StringMapToNative(values));
}

void SentryTransactionDesktop::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_remove_data(TransactionDesktop, TCHAR_TO_ANSI(*key));
}

void SentryTransactionDesktop::GetTrace(FString& name, FString& value)
{
	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_transaction_iter_headers(TransactionDesktop, CopyTransactionTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace")));

	sentry_value_decref(tracingHeader);
}

#endif
