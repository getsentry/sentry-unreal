// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionAndroid : public ISentryTransaction, public FSentryJavaObjectWrapper
{
public:
	SentryTransactionAndroid(jobject transaction);

	void SetupClassMethods();

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) override;
	virtual void Finish() override;

private:
	FSentryJavaMethod StartChildMethod;
	FSentryJavaMethod FinishMethod;
};
