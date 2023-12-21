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

	return SentryTransactionNativeImpl->StartChild(Operation, Description);
}

void USentryTransaction::Finish()
{
	if (!SentryTransactionNativeImpl || SentryTransactionNativeImpl->IsFinished())
		return;

	SentryTransactionNativeImpl->Finish();
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

void USentryTransaction::InitWithNativeImpl(TSharedPtr<ISentryTransaction> transactionImpl)
{
	SentryTransactionNativeImpl = transactionImpl;
}

TSharedPtr<ISentryTransaction> USentryTransaction::GetNativeImpl()
{
	return SentryTransactionNativeImpl;
}
