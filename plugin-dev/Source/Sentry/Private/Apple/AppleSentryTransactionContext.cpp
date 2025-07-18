// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryTransactionContext.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryTransactionContext::FAppleSentryTransactionContext(const FString& name, const FString& operation)
{
	TransactionContext = [[SENTRY_APPLE_CLASS(SentryTransactionContext) alloc] initWithName:name.GetNSString() operation:operation.GetNSString()];
}

FAppleSentryTransactionContext::FAppleSentryTransactionContext(SentryTransactionContext* context)
{
	TransactionContext = context;
}

FAppleSentryTransactionContext::~FAppleSentryTransactionContext()
{
	// Put custom destructor logic here if needed
}

FString FAppleSentryTransactionContext::GetName() const
{
	return FString(TransactionContext.name);
}

FString FAppleSentryTransactionContext::GetOperation() const
{
	return FString(TransactionContext.operation);
}

SentryTransactionContext* FAppleSentryTransactionContext::GetNativeObject()
{
	return TransactionContext;
}
