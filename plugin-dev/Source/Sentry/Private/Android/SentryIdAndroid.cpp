// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdAndroid.h"

SentryIdAndroid::SentryIdAndroid()
	: FSentryJavaObjectWrapper(GetClassName(), "()V")
{
	SetupClassMethods();
}

SentryIdAndroid::SentryIdAndroid(jobject id)
	: FSentryJavaObjectWrapper(GetClassName(), id)
{
	SetupClassMethods();
}

void SentryIdAndroid::SetupClassMethods()
{
	ToStringMethod = GetMethod("toString", "()Ljava/lang/String;");
}

FSentryJavaClass SentryIdAndroid::GetClassName()
{
	return FSentryJavaClass { "io/sentry/protocol/SentryId", ESentryJavaClassType::External };
}

FString SentryIdAndroid::ToString() const
{
	return CallMethod<FString>(ToStringMethod);
}
