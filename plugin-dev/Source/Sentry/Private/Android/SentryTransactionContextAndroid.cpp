// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionContextAndroid::SentryTransactionContextAndroid(jobject transactionContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, transactionContext)
{
	SetupClassMethods();
}

void SentryTransactionContextAndroid::SetupClassMethods()
{
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	GetOriginMethod = GetMethod("getOrigin", "()Ljava/lang/String;");
	GetOperationMethod = GetMethod("getOperation", "()Ljava/lang/String;");
}

FString SentryTransactionContextAndroid::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

FString SentryTransactionContextAndroid::GetOrigin() const
{
	return CallMethod<FString>(GetOriginMethod);
}

FString SentryTransactionContextAndroid::GetOperation() const
{
	return CallMethod<FString>(GetOperationMethod);
}
