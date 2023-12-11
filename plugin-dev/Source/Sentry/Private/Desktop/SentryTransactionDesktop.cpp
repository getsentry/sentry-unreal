// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionDesktop.h"

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

void SentryTransactionDesktop::Finish()
{
	sentry_transaction_finish(TransactionDesktop);
}

#endif
