// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryEventAndroid::SentryEventAndroid()
	: FSentryJavaObjectWrapper(GetClassName(), "()V")
{
	SetupClassMethods();
}

SentryEventAndroid::SentryEventAndroid(jobject event)
	: FSentryJavaObjectWrapper(GetClassName(), event)
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

FSentryJavaClass SentryEventAndroid::GetClassName()
{
	return FSentryJavaClass { "io/sentry/SentryEvent", ESentryJavaClassType::External };
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, SentryConvertorsAndroid::SentryMessageToNative(message)->GetJObject());
}

FString SentryEventAndroid::GetMessage() const
{
	auto message = NewSentryScopedJavaObject(CallMethod<jobject>(GetMessageMethod));
	return SentryConvertorsAndroid::SentryMessageToUnreal(*message);
}

void SentryEventAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel SentryEventAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}