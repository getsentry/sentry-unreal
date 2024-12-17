﻿// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySpanInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentrySpanAndroid : public ISentrySpan, public FSentryJavaObjectWrapper
{
public:
	SentrySpanAndroid(jobject span);

	void SetupClassMethods();

	virtual TSharedPtr<ISentrySpan> StartChild(const FString& operation, const FString& desctiption) override;
	virtual TSharedPtr<ISentrySpan> StartChildWithTimestamp(const FString& operation, const FString& desctiption, int64 timestamp) override;
	virtual void Finish() override;
	virtual void FinishWithTimestamp(int64 timestamp) override;
	virtual bool IsFinished() const override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveData(const FString& key) override;
	virtual void GetTrace(FString& name, FString& value) override;

private:
	FSentryJavaMethod StartChildMethod;
	FSentryJavaMethod FinishMethod;
	FSentryJavaMethod IsFinishedMethod;
	FSentryJavaMethod SetTagMethod;
	FSentryJavaMethod SetDataMethod;
	FSentryJavaMethod ToSentryTraceMethod;
};
