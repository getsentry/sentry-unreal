// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryMessage.h"

#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryMessage::FAndroidSentryMessage(const FString& message)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Message, "()V")
{
	SetupClassMethods();

	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

FAndroidSentryMessage::FAndroidSentryMessage(jobject message)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Message, message)
{
	SetupClassMethods();
}

void FAndroidSentryMessage::SetupClassMethods()
{
	SetMessageMethod = GetMethod("setMessage", "(Ljava/lang/String;)V");
	GetMessageMethod = GetMethod("getMessage", "()Ljava/lang/String;");
}

FString FAndroidSentryMessage::ToString() const
{
	return CallMethod<FString>(GetMessageMethod);
}