// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"

#include "SentryMessageAndroid.h"

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
	IsCrashMethod = GetMethod("isCrashed", "()Z");
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, SentryMessageAndroid(message).GetJObject());
}

FString SentryEventAndroid::GetMessage() const
{
	auto message = CallObjectMethod<jobject>(GetMessageMethod);
	return SentryMessageAndroid(*message).ToString();
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

bool SentryEventAndroid::IsCrash() const
{
	return CallMethod<bool>(IsCrashMethod);
}

bool SentryEventAndroid::IsAnr() const
{
	return CallStaticMethod<bool>(SentryJavaClasses::SentryBridgeJava, "isAnrEvent", "(Lio/sentry/SentryEvent;)Z", GetJObject());
}
