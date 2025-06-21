// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryTransaction.h"
#include "GenericPlatformSentrySpan.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

void CopyTransactionTracingHeader(const char* key, const char* value, void* userdata)
{
	sentry_value_t* header = static_cast<sentry_value_t*>(userdata);
	sentry_value_set_by_key(*header, key, sentry_value_new_string(value));
}

FGenericPlatformSentryTransaction::FGenericPlatformSentryTransaction(sentry_transaction_t* transaction)
	: Transaction(transaction)
	, isFinished(false)
{
}

sentry_transaction_t* FGenericPlatformSentryTransaction::GetNativeObject()
{
	return Transaction;
}

TSharedPtr<ISentrySpan> FGenericPlatformSentryTransaction::StartChildSpan(const FString& operation, const FString& description)
{
	if (sentry_span_t* nativeSpan = sentry_transaction_start_child(Transaction, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*description)))
	{
		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentrySpan> FGenericPlatformSentryTransaction::StartChildSpanWithTimestamp(const FString& operation, const FString& description, int64 timestamp)
{
	if (sentry_span_t* nativeSpan = sentry_transaction_start_child_ts(Transaction, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*description), timestamp))
	{
		return MakeShareable(new FGenericPlatformSentrySpan(nativeSpan));
	}
	else
	{
		return nullptr;
	}
}

void FGenericPlatformSentryTransaction::Finish()
{
	sentry_transaction_finish(Transaction);

	isFinished = true;
}

void FGenericPlatformSentryTransaction::FinishWithTimestamp(int64 timestamp)
{
	sentry_transaction_finish_ts(Transaction, timestamp);

	isFinished = true;
}

bool FGenericPlatformSentryTransaction::IsFinished() const
{
	return isFinished;
}

void FGenericPlatformSentryTransaction::SetName(const FString& name)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_name(Transaction, TCHAR_TO_ANSI(*name));
}

void FGenericPlatformSentryTransaction::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_tag(Transaction, TCHAR_TO_ANSI(*key), TCHAR_TO_ANSI(*value));
}

void FGenericPlatformSentryTransaction::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_remove_tag(Transaction, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentryTransaction::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_set_data(Transaction, TCHAR_TO_ANSI(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));
}

void FGenericPlatformSentryTransaction::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	sentry_transaction_remove_data(Transaction, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentryTransaction::GetTrace(FString& name, FString& value)
{
	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_transaction_iter_headers(Transaction, CopyTransactionTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace")));

	sentry_value_decref(tracingHeader);
}

#endif
