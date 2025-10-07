// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryLog.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryLog::FAndroidSentryLog()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryLogEvent, "()V")
{
	SetupClassMethods();
}

FAndroidSentryLog::FAndroidSentryLog(jobject logEvent)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryLogEvent, logEvent)
{
	SetupClassMethods();
}

FAndroidSentryLog::FAndroidSentryLog(const FString& body, ESentryLevel level)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryLogEvent, "()V")
{
	SetupClassMethods();

	SetBody(body);
	SetLevel(level);
}

void FAndroidSentryLog::SetupClassMethods()
{
	SetBodyMethod = GetMethod("setBody", "(Ljava/lang/String;)V");
	GetBodyMethod = GetMethod("getBody", "()Ljava/lang/String;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLogLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLogLevel;");
}

void FAndroidSentryLog::SetBody(const FString& body)
{
	CallMethod<void>(SetBodyMethod, *GetJString(body));
}

FString FAndroidSentryLog::GetBody() const
{
	return CallMethod<FString>(GetBodyMethod);
}

void FAndroidSentryLog::SetLevel(ESentryLevel level)
{
	TSharedPtr<FSentryJavaObjectWrapper> AndroidLevel = FAndroidSentryConverters::SentryLogLevelToNative(level);
	CallMethod<void>(SetLevelMethod, AndroidLevel->GetJObject());
}

ESentryLevel FAndroidSentryLog::GetLevel() const
{
	auto LevelObject = CallObjectMethod<jobject>(GetLevelMethod);
	return FAndroidSentryConverters::SentryLogLevelToUnreal(*LevelObject);
}