// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryTransactionContext : public ISentryTransactionContext, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryTransactionContext(const FString& name, const FString& operation);
	FAndroidSentryTransactionContext(jobject transactionContext);

	void SetupClassMethods();

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

private:
	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod GetOperationMethod;
};

typedef FAndroidSentryTransactionContext FPlatformSentryTransactionContext;
