// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContextAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionContextAndroid::SentryTransactionContextAndroid(jobject transactionContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionContext, transactionContext)
{
	SetupClassMethods();
}
