// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentrySpanInterface.h"

#if USE_SENTRY_NATIVE

class SentrySpanDesktop : public ISentrySpan
{
public:
	SentrySpanDesktop(sentry_span_t* span);
	virtual ~SentrySpanDesktop() override;

	sentry_span_t* GetNativeObject();

	virtual void Finish() override;

private:
	sentry_span_t* SpanDesktop;
};

#endif
