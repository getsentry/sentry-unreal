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
{
}

sentry_transaction_t* FGenericPlatformSentryTransaction::GetNativeObject()
{
	FScopeLock Lock(&CriticalSection);

	return Transaction;
}

TSharedPtr<ISentrySpan> FGenericPlatformSentryTransaction::StartChildSpan(const FString& operation, const FString& description, bool bindToScope)
{
	FScopeLock Lock(&CriticalSection);

	if (!Transaction)
	{
		return nullptr;
	}

	if (sentry_span_t* nativeSpan = sentry_transaction_start_child(Transaction, TCHAR_TO_UTF8(*operation), TCHAR_TO_UTF8(*description)))
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

TSharedPtr<ISentrySpan> FGenericPlatformSentryTransaction::StartChildSpanWithTimestamp(const FString& operation, const FString& description, int64 timestamp, bool bindToScope)
{
	FScopeLock Lock(&CriticalSection);

	if (!Transaction)
	{
		return nullptr;
	}

	if (sentry_span_t* nativeSpan = sentry_transaction_start_child_ts(Transaction, TCHAR_TO_UTF8(*operation), TCHAR_TO_UTF8(*description), timestamp))
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

void FGenericPlatformSentryTransaction::Finish()
{
	FScopeLock Lock(&CriticalSection);

	if (!Transaction)
	{
		return;
	}

	sentry_transaction_finish(Transaction);
	Transaction = nullptr;
}

void FGenericPlatformSentryTransaction::FinishWithTimestamp(int64 timestamp)
{
	FScopeLock Lock(&CriticalSection);

	if (!Transaction)
	{
		return;
	}

	sentry_transaction_finish_ts(Transaction, timestamp);
	Transaction = nullptr;
}

bool FGenericPlatformSentryTransaction::IsFinished() const
{
	FScopeLock Lock(&CriticalSection);

	return Transaction == nullptr;
}

void FGenericPlatformSentryTransaction::SetName(const FString& name)
{
	FScopeLock Lock(&CriticalSection);

	if (Transaction)
	{
		sentry_transaction_set_name(Transaction, TCHAR_TO_UTF8(*name));
	}
}

void FGenericPlatformSentryTransaction::SetTag(const FString& key, const FString& value)
{
	FScopeLock Lock(&CriticalSection);

	if (Transaction)
	{
		sentry_transaction_set_tag(Transaction, TCHAR_TO_UTF8(*key), TCHAR_TO_UTF8(*value));
	}
}

void FGenericPlatformSentryTransaction::RemoveTag(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	if (Transaction)
	{
		sentry_transaction_remove_tag(Transaction, TCHAR_TO_UTF8(*key));
	}
}

void FGenericPlatformSentryTransaction::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FScopeLock Lock(&CriticalSection);

	if (Transaction)
	{
		sentry_transaction_set_data(Transaction, TCHAR_TO_UTF8(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));
	}
}

void FGenericPlatformSentryTransaction::RemoveData(const FString& key)
{
	FScopeLock Lock(&CriticalSection);

	if (Transaction)
	{
		sentry_transaction_remove_data(Transaction, TCHAR_TO_UTF8(*key));
	}
}

void FGenericPlatformSentryTransaction::GetTrace(FString& name, FString& value)
{
	FScopeLock Lock(&CriticalSection);

	if (!Transaction)
	{
		return;
	}

	sentry_value_t tracingHeader = sentry_value_new_object();

	sentry_transaction_iter_headers(Transaction, CopyTransactionTracingHeader, &tracingHeader);

	name = TEXT("sentry-trace");
	value = FString(UTF8_TO_TCHAR(sentry_value_as_string(sentry_value_get_by_key(tracingHeader, "sentry-trace"))));

	sentry_value_decref(tracingHeader);
}

#endif
