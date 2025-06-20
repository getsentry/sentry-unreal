// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

@protocol SentrySpan;

class FAppleSentryTransaction : public ISentryTransaction
{
public:
	FAppleSentryTransaction(id<SentrySpan> transaction);
	virtual ~FAppleSentryTransaction() override;

	id<SentrySpan> GetNativeObject();

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
	id<SentrySpan> TransactionApple;
};

typedef FAppleSentryTransaction FPlatformSentryTransaction;
