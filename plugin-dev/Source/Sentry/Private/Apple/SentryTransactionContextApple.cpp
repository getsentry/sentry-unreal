// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextApple.h"

SentryTransactionContextApple::SentryTransactionContextApple(SentryTransactionContext* context)
{
	TransactionContext = context;
}

SentryTransactionContextApple::~SentryTransactionContextApple()
{
	// Put custom destructor logic here if needed
}

SentryTransactionContext* SentryTransactionContextApple::GetNativeObject()
{
	return TransactionContext;
}
