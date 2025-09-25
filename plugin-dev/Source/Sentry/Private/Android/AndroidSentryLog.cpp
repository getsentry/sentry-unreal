// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryLog.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryLog::FAndroidSentryLog()
{
	SetupClassMethods();

	// Create new SentryLogEvent Java object
	SetJObject(CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryLogEvent, "createLogEvent", "()Lio/sentry/SentryLogEvent;"));
}

FAndroidSentryLog::FAndroidSentryLog(jobject logEvent)
{
	SetupClassMethods();
	SetJObject(logEvent);
}

FAndroidSentryLog::FAndroidSentryLog(const FString& message, ESentryLevel level, const FString& category)
{
	SetupClassMethods();

	// Create new SentryLogEvent Java object
	SetJObject(CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryLogEvent, "createLogEvent", "()Lio/sentry/SentryLogEvent;"));

	SetMessage(message);
	SetLevel(level);
}

void FAndroidSentryLog::SetupClassMethods()
{
	SetMessageMethod = GetMethod("setMessage", "(Ljava/lang/String;)V");
	GetMessageMethod = GetMethod("getMessage", "()Ljava/lang/String;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
}

void FAndroidSentryLog::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

FString FAndroidSentryLog::GetMessage() const
{
	return CallMethod<FString>(GetMessageMethod);
}

void FAndroidSentryLog::SetLevel(ESentryLevel level)
{
	TSharedPtr<FAndroidSentryLevel> AndroidLevel = FAndroidSentryConverters::SentryLevelToNative(level);
	CallMethod<void>(SetLevelMethod, AndroidLevel->GetJObject());
}

ESentryLevel FAndroidSentryLog::GetLevel() const
{
	auto LevelObject = CallObjectMethod<jobject>(GetLevelMethod);
	TSharedPtr<FAndroidSentryLevel> AndroidLevel = MakeShareable(new FAndroidSentryLevel(LevelObject));
	return FAndroidSentryConverters::SentryLevelToUnreal(AndroidLevel);
}