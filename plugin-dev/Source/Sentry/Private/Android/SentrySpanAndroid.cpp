// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentrySpanAndroid::SentrySpanAndroid(jobject span)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Span, span)
{
}
