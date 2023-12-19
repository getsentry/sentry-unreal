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

USentrySpan* SentryTransactionApple::StartChild(const FString& operation, const FString& desctiption)
{
	id<SentrySpan> span = [TransactionApple startChildWithOperation:operation.GetNSString() description:desctiption.GetNSString()];
	return SentryConvertorsApple::SentrySpanToUnreal(span);
}

void SentryTransactionApple::Finish()
{
	[TransactionApple finish];
}

bool SentryTransactionApple::IsFinished()
{
	return TransactionApple.isFinished;
}

void SentryTransactionApple::SetName(const FString& name)
{
	[TransactionApple setDataValue:name.GetNSString() forKey:@"name"];
}

void SentryTransactionApple::SetTag(const FString& key, const FString& value)
{
	[TransactionApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void SentryTransactionApple::RemoveTag(const FString& key)
{
	[TransactionApple removeTagForKey:key.GetNSString()];
}

void SentryTransactionApple::SetData(const FString& key, const TMap<FString, FString>& values)
{
	[TransactionApple setDataValue:SentryConvertorsApple::StringMapToNative(values) forKey:key.GetNSString()];
}

void SentryTransactionApple::RemoveData(const FString& key)
{
	[TransactionApple removeDataForKey:key.GetNSString()];
}
