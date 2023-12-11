// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransactionApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryTransactionApple::SentryTransactionApple(id<SentrySpan> transaction)
{
	TransactionApple = transaction;
}

SentryTransactionApple::~SentryTransactionApple()
{
	// Put custom destructor logic here if needed
}

id<SentrySpan> SentryTransactionApple::GetNativeObject()
{
	return TransactionApple;
}

void SentryTransactionApple::Finish()
{
	[TransactionApple finish];
}
