// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryTransactionOptions : public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryTransactionOptions();

	void SetupClassMethods();

	void SetCustomSamplingContext(const TMap<FString, FString>& data);

private:
	FSentryJavaMethod SetCustomSamplingContextMethod;
};

typedef FAndroidSentryTransactionOptions FPlatformSentryTransactionOptions;
