// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryTransaction.h"
#include "SentryDefines.h"
#include "SentrySpan.h"

#include "HAL/PlatformSentryTransaction.h"

USentrySpan* USentryTransaction::StartChildSpan(const FString& Operation, const FString& Description)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	if (TSharedPtr<ISentrySpan> spanNativeImpl = NativeImpl->StartChildSpan(Operation, Description))
	{
		return USentrySpan::Create(spanNativeImpl);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Received invalid span after attempting to start child on transaction"));
		return nullptr;
	}
}

USentrySpan* USentryTransaction::StartChildSpanWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	if (TSharedPtr<ISentrySpan> spanNativeImpl = NativeImpl->StartChildSpanWithTimestamp(Operation, Description, Timestamp))
	{
		return USentrySpan::Create(spanNativeImpl);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Received invalid span after attempting to start child with timestamp on transaction"));
		return nullptr;
	}
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

void USentryTransaction::SetData(const FString& key, const TMap<FString, FSentryVariant>& values)
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
