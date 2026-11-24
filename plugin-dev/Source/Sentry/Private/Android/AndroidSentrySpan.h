// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySpanInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentrySpan : public ISentrySpan, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentrySpan(jobject span);

	void SetupClassMethods();

	virtual TSharedPtr<ISentrySpan> StartChild(const FString& operation, const FString& desctiption, bool bindToScope) override;
	virtual TSharedPtr<ISentrySpan> StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp, bool bindToScope) override;
	virtual void Finish() override;
	virtual void FinishWithTimestamp(int64 timestamp) override;
	virtual bool IsFinished() const override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FSentryVariant>& values) override;
	virtual void RemoveData(const FString& key) override;
	virtual void GetTrace(FString& name, FString& value) override;
	virtual TMap<FString, FString> GetTraceHeaders() override;

private:
	FSentryJavaMethod StartChildMethod;
	FSentryJavaMethod StartChildWithTimestampMethod;
	FSentryJavaMethod FinishMethod;
	FSentryJavaMethod FinishWithTimestampMethod;
	FSentryJavaMethod GetStatusMethod;
	FSentryJavaMethod IsFinishedMethod;
	FSentryJavaMethod SetTagMethod;
	FSentryJavaMethod SetDataMethod;
	FSentryJavaMethod ToSentryTraceMethod;
	FSentryJavaMethod ToBaggageHeaderMethod;
	FSentryJavaMethod IsNoOpMethod;
};

typedef FAndroidSentrySpan FPlatformSentrySpan;
