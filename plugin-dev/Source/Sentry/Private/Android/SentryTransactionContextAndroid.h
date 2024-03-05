// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionContextAndroid : public ISentryTransactionContext, public FSentryJavaObjectWrapper
{
public:
	SentryTransactionContextAndroid(const FString& name, const FString& operation);
	SentryTransactionContextAndroid(jobject transactionContext);

	void SetupClassMethods();

	virtual FString GetName() const override;
	virtual FString GetOperation() const override;

private:

	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod GetOperationMethod;
};
