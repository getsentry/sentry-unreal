// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryTransactionContextApple::SentryTransactionContextApple(const FString& name, const FString& operation)
{
	TransactionContext = [[SENTRY_APPLE_CLASS(SentryTransactionContext) alloc] initWithName:name.GetNSString() operation:operation.GetNSString()];
}

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

FString SentryTransactionContextApple::GetOperation() const
{
	return FString(TransactionContext.operation);
}

SentryTransactionContext* SentryTransactionContextApple::GetNativeObject()
{
	return TransactionContext;
}
