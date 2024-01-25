// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionContextAndroid : public ISentryTransactionContext, public FSentryJavaObjectWrapper
{
public:
	SentryTransactionContextAndroid(jobject transactionContext);

	void SetupClassMethods();

	virtual void SetName(const FString& Name) override;
	virtual FString GetName() const override;
	virtual void SetOrigin(const FString& Origin) override;
	virtual FString GetOrigin() const override;
	virtual void SetOperation(const FString& Operation) override;
	virtual FString GetOperation() const override;

private:
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod SetOriginMethod;
	FSentryJavaMethod GetOriginMethod;
	FSentryJavaMethod SetOperationMethod;
	FSentryJavaMethod GetOperationMethod;
};
