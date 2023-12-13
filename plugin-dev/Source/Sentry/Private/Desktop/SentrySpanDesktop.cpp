// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanDesktop.h"

#if USE_SENTRY_NATIVE

SentrySpanDesktop::SentrySpanDesktop(sentry_span_t* span)
	: SpanDesktop(span)
{
}

SentrySpanDesktop::~SentrySpanDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_span_t* SentrySpanDesktop::GetNativeObject()
{
	return SpanDesktop;
}

void SentrySpanDesktop::Finish()
{
	sentry_span_finish(SpanDesktop);
}

#endif
