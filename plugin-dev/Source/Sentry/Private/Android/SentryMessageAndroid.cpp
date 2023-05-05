// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryMessageAndroid.h"

#include "Infrastructure/SentryJavaClasses.h"

SentryMessageAndroid::SentryMessageAndroid(const FString& message)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Message, "()V")
{
	SetupClassMethods();

	CallMethod<void>(SetMessageMethod, *GetJString(message));
}

SentryMessageAndroid::SentryMessageAndroid(jobject message)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Message, message)
{
	SetupClassMethods();
}

void SentryMessageAndroid::SetupClassMethods()
{
	SetMessageMethod = GetMethod("setMessage", "(Ljava/lang/String;)V");
	GetMessageMethod = GetMethod("getMessage", "()Ljava/lang/String;");
}

FString SentryMessageAndroid::ToString() const
{
	return CallMethod<FString>(GetMessageMethod);
}