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

void SentryTransactionContextApple::SetName(const FString& Name)
{
	TransactionContext.name = Name.GetNSString();
}

FString SentryTransactionContextApple::GetName() const
{
	return FString(TransactionContext.name);
}

void SentryTransactionContextApple::SetOrigin(const FString& Origin)
{
	TransactionContext.origin = Origin.GetNSString();
}

FString SentryTransactionContextApple::GetOrigin() const
{
	return FString(TransactionContext.origin);
}

void SentryTransactionContextApple::SetOperation(const FString& Operation)
{
	TransactionContext.operation = Operation.GetNSString();
}

FString SentryTransactionContextApple::GetOperation() const
{
	return FString(TransactionContext.operation);
}

SentryTransactionContext* SentryTransactionContextApple::GetNativeObject()
{
	return TransactionContext;
}
