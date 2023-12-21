// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionDesktop.h"
#include "SentrySpanDesktop.h"

#include "SentrySpan.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

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

USentrySpan* SentryTransactionDesktop::StartChild(const FString& operation, const FString& desctiption)
{
	sentry_span_t* nativeSpan = sentry_transaction_start_child(TransactionDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption));
	return SentryConvertorsDesktop::SentrySpanToUnreal(nativeSpan);
}

void SentryTransactionDesktop::Finish()
{
	sentry_transaction_finish(TransactionDesktop);

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

#endif
