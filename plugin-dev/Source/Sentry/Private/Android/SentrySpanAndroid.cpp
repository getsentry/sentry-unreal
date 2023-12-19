// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpanAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentrySpanAndroid::SentrySpanAndroid(jobject span)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Span, span)
{
	SetupClassMethods();
}

void SentrySpanAndroid::SetupClassMethods()
{
	FinishMethod = GetMethod("finish", "()V");
}

void SentrySpanAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}

bool SentrySpanAndroid::IsFinished()
{
	return false;
}

void SentrySpanAndroid::SetTag(const FString& key, const FString& value)
{
}

void SentrySpanAndroid::RemoveTag(const FString& key)
{
}

void SentrySpanAndroid::SetData(const FString& key, const TMap<FString, FString>& values)
{
}

void SentrySpanAndroid::RemoveData(const FString& key)
{
}
