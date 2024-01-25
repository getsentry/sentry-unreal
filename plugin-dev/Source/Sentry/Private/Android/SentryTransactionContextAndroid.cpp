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
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	SetOriginMethod = GetMethod("setOrigin", "(Ljava/lang/String;)V");
	GetOriginMethod = GetMethod("getOrigin", "()Ljava/lang/String;");
	SetOperationMethod = GetMethod("setOperation", "(Ljava/lang/String;)V");
	GetOperationMethod = GetMethod("getOperation", "()Ljava/lang/String;");
}

void SentryTransactionContextAndroid::SetName(const FString& Name)
{
	CallMethod<void>(SetNameMethod, *GetJString(Name));
}

FString SentryTransactionContextAndroid::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void SentryTransactionContextAndroid::SetOrigin(const FString& Origin)
{
	CallMethod<void>(SetOriginMethod, *GetJString(Origin));
}

FString SentryTransactionContextAndroid::GetOrigin() const
{
	return CallMethod<FString>(GetOriginMethod);
}

void SentryTransactionContextAndroid::SetOperation(const FString& Operation)
{
	CallMethod<void>(SetOperationMethod, *GetJString(Operation));
}

FString SentryTransactionContextAndroid::GetOperation() const
{
	return CallMethod<FString>(GetOperationMethod);
}
