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

private:
	id<SentrySpan> SpanApple;
};
