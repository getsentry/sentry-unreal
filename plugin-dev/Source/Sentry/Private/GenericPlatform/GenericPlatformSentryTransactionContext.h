// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryTransactionContextInterface.h"

#if USE_SENTRY_NATIVE

class ISentryTransaction;

class FGenericPlatformSentryTransactionContext : public ISentryTransactionContext
{
public:
	FGenericPlatformSentryTransactionContext(const FString& name, const FString& operation);
	explicit FGenericPlatformSentryTransactionContext(const sentry_transaction_context_t* context);
	virtual ~FGenericPlatformSentryTransactionContext() override = default;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	void UpdateFromHeader(const FString& key, const FString& value);

	sentry_transaction_context_t* CreateNativeObject() const;

private:
	FString Name;
	FString Operation;

	TArray<TPair<FString, FString>> Headers;
};

typedef FGenericPlatformSentryTransactionContext FPlatformSentryTransactionContext;

#endif
