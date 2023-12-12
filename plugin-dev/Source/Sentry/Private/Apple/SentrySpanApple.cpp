// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanApple.h"

SentrySpanApple::SentrySpanApple(id<SentrySpan> span)
{
	SpanApple = span;
}

SentrySpanApple::~SentrySpanApple()
{
	// Put custom destructor logic here if needed
}

id<SentrySpan> SentrySpanApple::GetNativeObject()
{
	return SpanApple;
}

void SentrySpanApple::Finish()
{
	[SpanApple finish];
}
