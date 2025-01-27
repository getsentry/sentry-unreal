// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpan.h"

#include "Interface/SentrySpanInterface.h"

USentrySpan* USentrySpan::StartChild(const FString& Operation, const FString& Description)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	return USentrySpan::Create(NativeImpl->StartChild(Operation, Description));
}

USentrySpan* USentrySpan::StartChildWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return nullptr;

	return USentrySpan::Create(NativeImpl->StartChildWithTimestamp(Operation, Description, Timestamp));
}

void USentrySpan::Finish()
{
	if (!NativeImpl)
		return;

	NativeImpl->Finish();
}

void USentrySpan::FinishWithTimestamp(int64 Timestamp)
{
	if (!NativeImpl)
		return;

	NativeImpl->FinishWithTimestamp(Timestamp);
}

bool USentrySpan::IsFinished() const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->IsFinished();
}

void USentrySpan::SetTag(const FString& key, const FString& value)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->SetTag(key, value);
}

void USentrySpan::RemoveTag(const FString& key)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->RemoveTag(key);
}

void USentrySpan::SetData(const FString& key, const TMap<FString, FString>& values)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->SetData(key, values);
}

void USentrySpan::RemoveData(const FString& key)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->RemoveData(key);
}

void USentrySpan::GetTrace(FString& name, FString& value)
{
	if (!NativeImpl || NativeImpl->IsFinished())
		return;

	NativeImpl->GetTrace(name, value);
}
