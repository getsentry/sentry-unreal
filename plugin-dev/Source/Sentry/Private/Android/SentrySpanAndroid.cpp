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
	IsFinishedMethod = GetMethod("isFinished", "()Z");
	SetTagMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	SetDataMethod = GetMethod("setData", "(Ljava/lang/String;Ljava/lang/Object;)V");
}

void SentrySpanAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}

bool SentrySpanAndroid::IsFinished() const
{
	return CallMethod<bool>(IsFinishedMethod);;
}

void SentrySpanAndroid::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagMethod, *GetJString(key), *GetJString(value));
}

void SentrySpanAndroid::RemoveTag(const FString& key)
{
	SetTag(key, TEXT(""));
}

void SentrySpanAndroid::SetData(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetDataMethod, *GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentrySpanAndroid::RemoveData(const FString& key)
{
	SetData(key, TMap<FString, FString>());
}
