// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionContextInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionContextAndroid : public ISentryTransactionContext, public FSentryJavaObjectWrapper
{
public:
	SentryTransactionContextAndroid(jobject transactionContext);

	void SetupClassMethods();

private:
	FSentryJavaMethod SomeMethod;
};
