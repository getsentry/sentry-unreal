// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "HAL/CriticalSection.h"

#include "Interface/SentrySpanInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentrySpan : public ISentrySpan
{
public:
	FGenericPlatformSentrySpan(sentry_span_t* span);
	virtual ~FGenericPlatformSentrySpan() override = default;

	sentry_span_t* GetNativeObject();

	virtual TSharedPtr<ISentrySpan> StartChild(const FString& operation, const FString& description) override;
	virtual TSharedPtr<ISentrySpan> StartChildWithTimestamp(const FString& operation, const FString& description, int64 timestamp) override;
	virtual void Finish() override;
	virtual void FinishWithTimestamp(int64 timestamp) override;
	virtual bool IsFinished() const override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FSentryVariant>& values) override;
	virtual void RemoveData(const FString& key) override;
	virtual void GetTrace(FString& name, FString& value) override;

private:
	sentry_span_t* Span;

	FCriticalSection CriticalSection;

	bool isFinished;
};

typedef FGenericPlatformSentrySpan FPlatformSentrySpan;

#endif
