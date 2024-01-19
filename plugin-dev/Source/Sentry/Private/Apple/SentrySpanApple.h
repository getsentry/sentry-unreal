// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySpanInterface.h"

@protocol SentrySpan;

class SentrySpanApple : public ISentrySpan
{
public:
	SentrySpanApple(id<SentrySpan> span);
	virtual ~SentrySpanApple() override;

	id<SentrySpan> GetNativeObject();

	virtual void Finish() override;
	virtual bool IsFinished() const override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetData(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveData(const FString& key) override;


private:
	id<SentrySpan> SpanApple;
};
