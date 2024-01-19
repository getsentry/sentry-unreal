// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryTransactionInterface.h"

@protocol SentrySpan;

class SentryTransactionApple : public ISentryTransaction
{
public:
	SentryTransactionApple(id<SentrySpan> transaction);
	virtual ~SentryTransactionApple() override;

	id<SentrySpan> GetNativeObject();

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) override;
	virtual void Finish() override;
	virtual bool IsFinished() const override;
	virtual void SetName(const FString& name) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveData(const FString& key) override;

private:
	id<SentrySpan> TransactionApple;
};
