// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryTransactionOptionsAndroid : public FSentryJavaObjectWrapper
{
public:
	SentryTransactionOptionsAndroid();

	void SetupClassMethods();

	void SetCustomSamplingContext(const TMap<FString, FString>& data);

private:
	FSentryJavaMethod SetCustomSamplingContextMethod;
};
