// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryTransactionContext.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryTransactionContext::FAndroidSentryTransactionContext(const FString& name, const FString& operation)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, "(Ljava/lang/String;Ljava/lang/String;)V",
		  *GetJString(name), *GetJString(operation))
{
	SetupClassMethods();
}

FAndroidSentryTransactionContext::FAndroidSentryTransactionContext(jobject transactionContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, transactionContext)
{
	SetupClassMethods();
}

void FAndroidSentryTransactionContext::SetupClassMethods()
{
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	GetOperationMethod = GetMethod("getOperation", "()Ljava/lang/String;");
}

FString FAndroidSentryTransactionContext::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

FString FAndroidSentryTransactionContext::GetOperation() const
{
	return CallMethod<FString>(GetOperationMethod);
}
