// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryEventAndroid::SentryEventAndroid()
	: FSentryJavaClassWrapper(GetClassName(), "()V")
{
	SetupClassMethods();
}

SentryEventAndroid::SentryEventAndroid(jobject event)
	: FSentryJavaClassWrapper(GetClassName(), event)
{
	SetupClassMethods();
}

void SentryEventAndroid::SetupClassMethods()
{
	SetMessageMethod = GetClassMethod("setMessage", "(Lio/sentry/protocol/Message;)V");
	GetMessageMethod = GetClassMethod("getMessage", "()Lio/sentry/protocol/Message;");
	SetLevelMethod = GetClassMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetClassMethod("getLevel", "()Lio/sentry/SentryLevel;");
}

FName SentryEventAndroid::GetClassName()
{
	return FName("io/sentry/SentryEvent");
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, SentryConvertorsAndroid::SentryMessageToNative(message));
}

FString SentryEventAndroid::GetMessage() const
{
	auto message = NewSentryScopedJavaObject(CallMethod<jobject>(GetMessageMethod));
	return SentryConvertorsAndroid::SentryMessageToUnreal(*message);
}

void SentryEventAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryEventAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}