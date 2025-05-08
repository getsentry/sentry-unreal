// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "AndroidSentryEvent.h"
#include "AndroidSentryId.h"
#include "AndroidSentryMessage.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryEvent::FAndroidSentryEvent()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, "()V")
{
	SetupClassMethods();
}

FAndroidSentryEvent::FAndroidSentryEvent(jobject event)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, event)
{
	SetupClassMethods();
}

void FAndroidSentryEvent::SetupClassMethods()
{
	GetIdMethod = GetMethod("getEventId", "()Lio/sentry/protocol/SentryId;");
	SetMessageMethod = GetMethod("setMessage", "(Lio/sentry/protocol/Message;)V");
	GetMessageMethod = GetMethod("getMessage", "()Lio/sentry/protocol/Message;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
	IsCrashMethod = GetMethod("isCrashed", "()Z");
}

TSharedPtr<ISentryId> FAndroidSentryEvent::GetId() const
{
	auto id = CallObjectMethod<jobject>(GetIdMethod);
	return MakeShareable(new FAndroidSentryId(*id));
}

void FAndroidSentryEvent::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, FAndroidSentryMessage(message).GetJObject());
}

FString FAndroidSentryEvent::GetMessage() const
{
	auto message = CallObjectMethod<jobject>(GetMessageMethod);
	return FAndroidSentryMessage(*message).ToString();
}

void FAndroidSentryEvent::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel FAndroidSentryEvent::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return FAndroidSentryConverters::SentryLevelToUnreal(*level);
}

bool FAndroidSentryEvent::IsCrash() const
{
	return CallMethod<bool>(IsCrashMethod);
}

bool FAndroidSentryEvent::IsAnr() const
{
	return CallStaticMethod<bool>(SentryJavaClasses::SentryBridgeJava, "isAnrEvent", "(Lio/sentry/SentryEvent;)Z", GetJObject());
}
