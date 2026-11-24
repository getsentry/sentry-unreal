// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentryTransactionContextInterface.h"

@class SentryObjCTransactionContext;

class FAppleSentryTransactionContext : public ISentryTransactionContext
{
public:
	FAppleSentryTransactionContext(const FString& name, const FString& operation);
	FAppleSentryTransactionContext(SentryObjCTransactionContext* context);
	virtual ~FAppleSentryTransactionContext() override;

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

	SentryObjCTransactionContext* GetNativeObject();

private:
	SentryObjCTransactionContext* TransactionContext;
};

typedef FAppleSentryTransactionContext FPlatformSentryTransactionContext;

#endif // !USE_SENTRY_NATIVE
