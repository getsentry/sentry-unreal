// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryId.h"

#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryId::FAndroidSentryId()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryId, "()V")
{
	SetupClassMethods();
}

FAndroidSentryId::FAndroidSentryId(const FString& id)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryId, "(Ljava/lang/String;)V", *GetJString(id))
{
	SetupClassMethods();
}

FAndroidSentryId::FAndroidSentryId(jobject id)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryId, id)
{
	SetupClassMethods();
}

void FAndroidSentryId::SetupClassMethods()
{
	ToStringMethod = GetMethod("toString", "()Ljava/lang/String;");
}

FString FAndroidSentryId::ToString() const
{
	return CallMethod<FString>(ToStringMethod);
}
