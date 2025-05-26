// Copyright (c) 2025 Sentry. All Rights Reserved.

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
	SetFingerprintMethod = GetMethod("setFingerprints", "(Ljava/util/List;)V");
	GetFingerprintMethod = GetMethod("getFingerprints()", "()Ljava/util/List;");
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

void FAndroidSentryEvent::SetFingerprint(const TArray<FString>& fingerprint)
{
	CallMethod<void>(SetFingerprintMethod, FAndroidSentryConverters::StringArrayToNative(fingerprint)->GetJObject());
}

TArray<FString> FAndroidSentryEvent::GetFingerprint()
{
	auto fingerprint = CallObjectMethod<jobject>(GetFingerprintMethod);
	return FAndroidSentryConverters::StringListToUnreal(*fingerprint);
}

bool FAndroidSentryEvent::IsCrash() const
{
	return CallMethod<bool>(IsCrashMethod);
}

bool FAndroidSentryEvent::IsAnr() const
{
	return CallStaticMethod<bool>(SentryJavaClasses::SentryBridgeJava, "isAnrEvent", "(Lio/sentry/SentryEvent;)Z", GetJObject());
}
