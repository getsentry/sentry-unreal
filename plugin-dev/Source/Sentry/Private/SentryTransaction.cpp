// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransaction.h"
#include "SentrySpan.h"

#include "Interface/SentryTransactionInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryTransactionAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryTransactionApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryTransactionDesktop.h"
#endif

USentryTransaction::USentryTransaction()
{
}

USentrySpan* USentryTransaction::StartChild(const FString& Operation, const FString& Description)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = SentryTransactionNativeImpl->StartChild(Operation, Description);

	USentrySpan* unrealSpan = NewObject<USentrySpan>();
	unrealSpan->InitWithNativeImpl(spanNativeImpl);

	return unrealSpan;
}

USentrySpan* USentryTransaction::StartChildWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = SentryTransactionNativeImpl->StartChildWithTimestamp(Operation, Description, Timestamp);

	USentrySpan* unrealSpan = NewObject<USentrySpan>();
	unrealSpan->InitWithNativeImpl(spanNativeImpl);

	return unrealSpan;
}

void USentryTransaction::Finish()
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->Finish();
}

void USentryTransaction::FinishWithTimestamp(int64 Timestamp)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->FinishWithTimestamp(Timestamp);
}

bool USentryTransaction::IsFinished() const
{
	if (!SentryTransactionNativeImpl)
		return false;

	return SentryTransactionNativeImpl->IsFinished();
}

void USentryTransaction::SetName(const FString& name)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->SetName(name);
}

void USentryTransaction::SetTag(const FString& key, const FString& value)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->SetTag(key, value);
}

void USentryTransaction::RemoveTag(const FString& key)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->RemoveTag(key);
}

void USentryTransaction::SetData(const FString& key, const TMap<FString, FString>& values)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->SetData(key, values);
}

void USentryTransaction::RemoveData(const FString& key)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->RemoveData(key);
}

void USentryTransaction::GetTrace(FString& name, FString& value)
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->GetTrace(name, value);
}

void USentryTransaction::InitWithNativeImpl(TSharedPtr<ISentryTransaction> transactionImpl)
{
	SentryTransactionNativeImpl = transactionImpl;
}

TSharedPtr<ISentryTransaction> USentryTransaction::GetNativeImpl()
{
	return SentryTransactionNativeImpl;
}
