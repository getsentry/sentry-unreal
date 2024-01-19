// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "HAL/CriticalSection.h"

#include "Interface/SentryTransactionInterface.h"

#if USE_SENTRY_NATIVE

class SentryTransactionDesktop : public ISentryTransaction
{
public:
	SentryTransactionDesktop(sentry_transaction_t* transaction);
	virtual ~SentryTransactionDesktop() override;

	sentry_transaction_t* GetNativeObject();

	virtual USentrySpan* StartChild(const FString& operation, const FString& desctiption) override;
	virtual void Finish() override;
	virtual bool IsFinished() const override;
	virtual void SetName(const FString& name) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveData(const FString& key) override;

private:
	sentry_transaction_t* TransactionDesktop;

	FCriticalSection CriticalSection;

	bool isFinished;
};

#endif
