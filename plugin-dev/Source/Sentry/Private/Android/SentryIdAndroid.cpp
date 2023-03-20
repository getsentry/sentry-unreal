// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdAndroid.h"

SentryIdAndroid::SentryIdAndroid()
	: FSentryJavaClassWrapper(GetClassName(), "()V")
{
	SetupClassMethods();
}

SentryIdAndroid::SentryIdAndroid(jobject id)
	: FSentryJavaClassWrapper(GetClassName(), id)
{
	SetupClassMethods();
}

void SentryIdAndroid::SetupClassMethods()
{
	ToStringMethod = GetClassMethod("toString", "()Ljava/lang/String;");
}

FName SentryIdAndroid::GetClassName()
{
	return FName("io/sentry/protocol/SentryId");
}

FString SentryIdAndroid::ToString() const
{
	return CallMethod<FString>(ToStringMethod);
}
