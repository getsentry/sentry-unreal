// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryEventAndroid::SentryEventAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, "()V")
{
	SetupClassMethods();
}

SentryEventAndroid::SentryEventAndroid(jobject event)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, event)
{
	SetupClassMethods();
}

void SentryEventAndroid::SetupClassMethods()
{
	SetMessageMethod = GetMethod("setMessage", "(Lio/sentry/protocol/Message;)V");
	GetMessageMethod = GetMethod("getMessage", "()Lio/sentry/protocol/Message;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, SentryConvertorsAndroid::SentryMessageToNative(message)->GetJObject());
}

FString SentryEventAndroid::GetMessage() const
{
	auto message = CallObjectMethod<jobject>(GetMessageMethod);
	return SentryConvertorsAndroid::SentryMessageToUnreal(*message);
}

void SentryEventAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel SentryEventAndroid::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}