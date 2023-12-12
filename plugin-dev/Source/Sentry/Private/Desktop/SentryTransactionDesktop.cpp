// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionDesktop.h"

#include "SentrySpan.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentryTransactionDesktop::SentryTransactionDesktop(sentry_transaction_t* transaction)
	: TransactionDesktop(transaction)
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
	sentry_span_t* span = sentry_transaction_start_child(TransactionDesktop, TCHAR_TO_ANSI(*operation), TCHAR_TO_ANSI(*desctiption));
	return nullptr;
}

void SentryTransactionDesktop::Finish()
{
	sentry_transaction_finish(TransactionDesktop);
}

#endif
