// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdAndroid.h"

#include "Infrastructure/SentryJavaClasses.h"

SentryIdAndroid::SentryIdAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryId, "()V")
{
	SetupClassMethods();
}

SentryIdAndroid::SentryIdAndroid(jobject id)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryId, id)
{
	SetupClassMethods();
}

void SentryIdAndroid::SetupClassMethods()
{
	ToStringMethod = GetMethod("toString", "()Ljava/lang/String;");
}

FString SentryIdAndroid::ToString() const
{
	return CallMethod<FString>(ToStringMethod);
}
