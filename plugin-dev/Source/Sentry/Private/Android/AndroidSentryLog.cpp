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

void FAndroidSentryLog::SetAttribute(const FString& key, const FSentryVariant& value)
{
	// TODO: Implement when Android SDK supports structured logging with attributes
}

FSentryVariant FAndroidSentryLog::GetAttribute(const FString& key) const
{
	// TODO: Implement when Android SDK supports structured logging with attributes
	return FSentryVariant();
}

bool FAndroidSentryLog::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	// TODO: Implement when Android SDK supports structured logging with attributes
	return false;
}

void FAndroidSentryLog::RemoveAttribute(const FString& key)
{
	// TODO: Implement when Android SDK supports structured logging with attributes
}

void FAndroidSentryLog::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	// TODO: Implement when Android SDK supports structured logging with attributes
}