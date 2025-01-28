// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "GenericPlatformSentryTransactionContext.h"
#include "GenericPlatformSentryTransaction.h"

#include "SentryDefines.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(const FString& name, const FString& operation)
	: TransactionContext(sentry_transaction_context_new(TCHAR_TO_ANSI(*name), TCHAR_TO_ANSI(*operation)))
{
}

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(sentry_transaction_context_t* context)
	: TransactionContext(context)
{
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentryTransactionContext::StartTransaction() const
{
	if (sentry_transaction_t* nativeTransaction = sentry_transaction_start(TransactionContext, sentry_value_new_null()))
	{
		return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentryTransactionContext::StartTransactionWithTimestamp(int64 timestamp) const
{
	if (sentry_transaction_t* nativeTransaction = sentry_transaction_start_ts(TransactionContext, sentry_value_new_null(), timestamp))
	{
		return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
	}
	else
	{
		return nullptr;
	}
}

FString FGenericPlatformSentryTransactionContext::GetName() const
{
	// no corresponding implementation in sentry-native
	UE_LOG(LogSentrySdk, Warning, TEXT("The native SDK doesn't currently support transaction's context GetName function"));
	return FString();
}

FString FGenericPlatformSentryTransactionContext::GetOperation() const
{
	// no corresponding implementation in sentry-native
	UE_LOG(LogSentrySdk, Warning, TEXT("The native SDK doesn't currently support transaction's context GetOperation function"));
	return FString();
}

sentry_transaction_context_t* FGenericPlatformSentryTransactionContext::GetNativeObject()
{
	return TransactionContext;
}

#endif
