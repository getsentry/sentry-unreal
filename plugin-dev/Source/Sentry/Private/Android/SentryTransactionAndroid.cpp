// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionAndroid::SentryTransactionAndroid(jobject transaction)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Transaction, transaction)
{
	SetupClassMethods();
}

void SentryTransactionAndroid::SetupClassMethods()
{
	StartChildMethod = GetMethod("startChild", "(Ljava/lang/String;Ljava/lang/String;)Lsentry/io/ISpan;");
	FinishMethod = GetMethod("finish", "()V");
}

USentrySpan* SentryTransactionAndroid::StartChild(const FString& operation, const FString& desctiption)
{
	auto span = CallObjectMethod<jobject>(StartChildMethod, *GetJString(operation), *GetJString(desctiption));
	return SentryConvertorsAndroid::SentrySpanToUnreal(*span);
}

void SentryTransactionAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}
