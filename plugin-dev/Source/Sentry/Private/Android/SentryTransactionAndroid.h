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
	virtual bool IsFinished() const override;
	virtual void SetName(const FString& name) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveData(const FString& key) override;

private:
	FSentryJavaMethod StartChildMethod;
	FSentryJavaMethod FinishMethod;
	FSentryJavaMethod IsFinishedMethod;
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod SetTagMethod;
	FSentryJavaMethod SetDataMethod;
};
