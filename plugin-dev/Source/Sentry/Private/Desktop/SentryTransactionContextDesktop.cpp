// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextDesktop.h"

#include "SentryDefines.h"

#if USE_SENTRY_NATIVE

SentryTransactionContextDesktop::SentryTransactionContextDesktop(const FString& name, const FString& operation)
	: TransactionContextDesktop(sentry_transaction_context_new(TCHAR_TO_ANSI(*name), TCHAR_TO_ANSI(*operation)))
{
}

SentryTransactionContextDesktop::SentryTransactionContextDesktop(sentry_transaction_context_t* context)
	: TransactionContextDesktop(context)
{
}

SentryTransactionContextDesktop::~SentryTransactionContextDesktop()
{
	// Put custom destructor logic here if needed
}

FString SentryTransactionContextDesktop::GetName() const
{
	// no corresponding implementation in sentry-native
	UE_LOG(LogSentrySdk, Warning, TEXT("The native SDK doesn't currently support transaction's context GetName function"));
	return FString();
}

FString SentryTransactionContextDesktop::GetOperation() const
{
	// no corresponding implementation in sentry-native
	UE_LOG(LogSentrySdk, Warning, TEXT("The native SDK doesn't currently support transaction's context GetOperation function"));
	return FString();
}

sentry_transaction_context_t* SentryTransactionContextDesktop::GetNativeObject()
{
	return TransactionContextDesktop;
}

#endif
