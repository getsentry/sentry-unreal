// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySpanInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentrySpanAndroid : public ISentrySpan, public FSentryJavaObjectWrapper
{
public:
	SentrySpanAndroid(jobject span);

	void SetupClassMethods();

	virtual void Finish() override;

private:
	FSentryJavaMethod FinishMethod;
};
