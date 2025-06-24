// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryTransaction : public ISentryTransaction, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryTransaction(jobject transaction);

	void SetupClassMethods();

	virtual TSharedPtr<ISentrySpan> StartChildSpan(const FString& operation, const FString& desctiption) override;
	virtual TSharedPtr<ISentrySpan> StartChildSpanWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp) override;
	virtual void Finish() override;
	virtual void FinishWithTimestamp(int64 timestamp) override;
	virtual bool IsFinished() const override;
	virtual void SetName(const FString& name) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FSentryVariant>& values) override;
	virtual void RemoveData(const FString& key) override;
	virtual void GetTrace(FString& name, FString& value) override;

private:
	FSentryJavaMethod StartChildMethod;
	FSentryJavaMethod FinishMethod;
	FSentryJavaMethod IsFinishedMethod;
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod SetTagMethod;
	FSentryJavaMethod SetDataMethod;
	FSentryJavaMethod ToSentryTraceMethod;
};

typedef FAndroidSentryTransaction FPlatformSentryTransaction;
