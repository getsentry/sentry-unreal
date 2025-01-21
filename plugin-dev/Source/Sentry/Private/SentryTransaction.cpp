// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransaction.h"
#include "SentrySpan.h"

#include "Interface/SentryTransactionInterface.h"

USentrySpan* USentryTransaction::StartChild(const FString& Operation, const FString& Description)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = NativeImpl->StartChild(Operation, Description);

	return USentrySpan::Create(spanNativeImpl);
}

USentrySpan* USentryTransaction::StartChildWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = NativeImpl->StartChildWithTimestamp(Operation, Description, Timestamp);

	return USentrySpan::Create(spanNativeImpl);
}

void USentryTransaction::Finish()
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->Finish();
}

void USentryTransaction::FinishWithTimestamp(int64 Timestamp)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->FinishWithTimestamp(Timestamp);
}

bool USentryTransaction::IsFinished() const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->IsFinished();
}

void USentryTransaction::SetName(const FString& name)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->SetName(name);
}

void USentryTransaction::SetTag(const FString& key, const FString& value)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->SetTag(key, value);
}

void USentryTransaction::RemoveTag(const FString& key)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->RemoveTag(key);
}

void USentryTransaction::SetData(const FString& key, const TMap<FString, FString>& values)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->SetData(key, values);
}

void USentryTransaction::RemoveData(const FString& key)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->RemoveData(key);
}

void USentryTransaction::GetTrace(FString& name, FString& value)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->GetTrace(name, value);
}
