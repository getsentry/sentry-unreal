// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionAndroid : public ISentryTransaction, public FSentryJavaObjectWrapper
{
public:
	SentryTransactionAndroid(jobject transaction);

	void SetupClassMethods();

	virtual void Finish() override;

private:
	FSentryJavaMethod FinishMethod;
};
