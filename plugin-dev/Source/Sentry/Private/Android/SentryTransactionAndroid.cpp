// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionAndroid::SentryTransactionAndroid(jobject transaction)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Transaction, transaction)
{
}

void SentryTransactionAndroid::SetupClassMethods()
{
	FinishMethod = GetMethod("finish", "()V");
}

void SentryTransactionAndroid::Finish()
{
	CallMethod<void>(FinishMethod);
}
