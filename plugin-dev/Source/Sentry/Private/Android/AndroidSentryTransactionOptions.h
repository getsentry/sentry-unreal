// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryVariant.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryTransactionOptions : public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryTransactionOptions();

	void SetupClassMethods();

	void SetCustomSamplingContext(const TMap<FString, FSentryVariant>& data);
	void SetBindToScope(bool bindToScope);
	void SetStartTimestamp(int64 timestamp);

private:
	FSentryJavaMethod SetCustomSamplingContextMethod;
	FSentryJavaMethod SetBindToScopeMethod;
	FSentryJavaMethod SetStartTimestampMethod;
};

typedef FAndroidSentryTransactionOptions FPlatformSentryTransactionOptions;
