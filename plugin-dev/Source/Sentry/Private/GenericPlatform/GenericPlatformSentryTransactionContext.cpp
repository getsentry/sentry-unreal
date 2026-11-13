// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryTransactionContext.h"

#include "SentryDefines.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(const FString& name, const FString& operation)
	: Name(name)
	, Operation(operation)
{
}

FGenericPlatformSentryTransactionContext::FGenericPlatformSentryTransactionContext(const sentry_transaction_context_t* context)
	: Name(UTF8_TO_TCHAR(sentry_transaction_context_get_name(context)))
	, Operation(UTF8_TO_TCHAR(sentry_transaction_context_get_operation(context)))
{
}

FString FGenericPlatformSentryTransactionContext::GetName() const
{
	return Name;
}

FString FGenericPlatformSentryTransactionContext::GetOperation() const
{
	return Operation;
}

void FGenericPlatformSentryTransactionContext::UpdateFromHeader(const FString& key, const FString& value)
{
	Headers.Emplace(key, value);
}

sentry_transaction_context_t* FGenericPlatformSentryTransactionContext::CreateNativeObject() const
{
	sentry_transaction_context_t* context = sentry_transaction_context_new(TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(*Operation));

	for (const TPair<FString, FString>& Header : Headers)
	{
		sentry_transaction_context_update_from_header(context, TCHAR_TO_UTF8(*Header.Key), TCHAR_TO_UTF8(*Header.Value));
	}

	return context;
}

#endif
