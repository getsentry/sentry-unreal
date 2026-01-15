// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryTransactionContext.h"
#include "GenericPlatformSentryTransaction.h"

#include "SentryDefines.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(const FString& name, const FString& operation)
	: TransactionContext(sentry_transaction_context_new(TCHAR_TO_UTF8(*name), TCHAR_TO_UTF8(*operation)))
{
}

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(sentry_transaction_context_t* context)
	: TransactionContext(context)
{
}

FString FGenericPlatformSentryTransactionContext::GetName() const
{
	return FString(UTF8_TO_TCHAR(sentry_transaction_context_get_name(TransactionContext)));
}

FString FGenericPlatformSentryTransactionContext::GetOperation() const
{
	return FString(UTF8_TO_TCHAR(sentry_transaction_context_get_operation(TransactionContext)));
}

sentry_transaction_context_t* FGenericPlatformSentryTransactionContext::GetNativeObject()
{
	return TransactionContext;
}

#endif
