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

FString SentryTransactionContextApple::GetName() const
{
	return FString(TransactionContext.name);
}

FString SentryTransactionContextApple::GetOrigin() const
{
	return FString(TransactionContext.origin);
}

FString SentryTransactionContextApple::GetOperation() const
{
	return FString(TransactionContext.operation);
}

SentryTransactionContext* SentryTransactionContextApple::GetNativeObject()
{
	return TransactionContext;
}
