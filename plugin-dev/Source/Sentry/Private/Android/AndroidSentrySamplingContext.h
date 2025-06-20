// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySamplingContextInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentrySamplingContext : public ISentrySamplingContext, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentrySamplingContext(jobject samplingContext);

	void SetupClassMethods();

	virtual TSharedPtr<ISentryTransactionContext> GetTransactionContext() const override;
	virtual TMap<FString, FString> GetCustomSamplingContext() const override;

private:
	FSentryJavaMethod GetTransactionContextMethod;
	FSentryJavaMethod GetCustomSamplingContextMethod;
};

typedef FAndroidSentrySamplingContext FPlatformSentrySamplingContext;