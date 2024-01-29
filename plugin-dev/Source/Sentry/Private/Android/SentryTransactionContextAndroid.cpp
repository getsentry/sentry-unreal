// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionContextAndroid::SentryTransactionContextAndroid(const FString& name, const FString& operation)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, "(Ljava/lang/String;Ljava/lang/String;)V",
		*GetJString(name), *GetJString(operation))
{
	SetupClassMethods();
}

SentryTransactionContextAndroid::SentryTransactionContextAndroid(jobject transactionContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, transactionContext)
{
	SetupClassMethods();
}

void SentryTransactionContextAndroid::SetupClassMethods()
{
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	GetOperationMethod = GetMethod("getOperation", "()Ljava/lang/String;");
	SetOriginMethod = GetMethod("setOrigin", "(Ljava/lang/String;)V");
	GetOriginMethod = GetMethod("getOrigin", "()Ljava/lang/String;");
}

FString SentryTransactionContextAndroid::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

FString SentryTransactionContextAndroid::GetOperation() const
{
	return CallMethod<FString>(GetOperationMethod);
}

void SentryTransactionContextAndroid::SetOrigin(const FString& Origin)
{
	CallMethod<void>(SetOriginMethod, *GetJString(Origin));
}

FString SentryTransactionContextAndroid::GetOrigin() const
{
	return CallMethod<FString>(GetOriginMethod);
}
