// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentrySpan.h"

#include "Interface/SentrySpanInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentrySpanAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySpanApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentrySpanDesktop.h"
#endif

USentrySpan::USentrySpan()
{
}

USentrySpan* USentrySpan::StartChild(const FString& Operation, const FString& Description)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = SentrySpanNativeImpl->StartChild(Operation, Description);

	USentrySpan* unrealSpan = NewObject<USentrySpan>();
	unrealSpan->InitWithNativeImpl(spanNativeImpl);

	return unrealSpan;
}

USentrySpan* USentrySpan::StartChildWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return nullptr;

	TSharedPtr<ISentrySpan> spanNativeImpl = SentrySpanNativeImpl->StartChildWithTimestamp(Operation, Description, Timestamp);

	USentrySpan* unrealSpan = NewObject<USentrySpan>();
	unrealSpan->InitWithNativeImpl(spanNativeImpl);

	return unrealSpan;
}

void USentrySpan::Finish()
{
	if (!SentrySpanNativeImpl)
		return;

	SentrySpanNativeImpl->Finish();
}

void USentrySpan::FinishWithTimestamp(int64 Timestamp)
{
	if (!SentrySpanNativeImpl)
		return;

	SentrySpanNativeImpl->FinishWithTimestamp(Timestamp);
}

bool USentrySpan::IsFinished() const
{
	if (!SentrySpanNativeImpl)
		return false;

	return SentrySpanNativeImpl->IsFinished();
}

void USentrySpan::SetTag(const FString& key, const FString& value)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return;

	SentrySpanNativeImpl->SetTag(key, value);
}

void USentrySpan::RemoveTag(const FString& key)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return;

	SentrySpanNativeImpl->RemoveTag(key);
}

void USentrySpan::SetData(const FString& key, const TMap<FString, FString>& values)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return;

	SentrySpanNativeImpl->SetData(key, values);
}

void USentrySpan::RemoveData(const FString& key)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return;

	SentrySpanNativeImpl->RemoveData(key);
}

void USentrySpan::GetTrace(FString& name, FString& value)
{
	if (!SentrySpanNativeImpl || SentrySpanNativeImpl->IsFinished())
		return;

	SentrySpanNativeImpl->GetTrace(name, value);
}

void USentrySpan::InitWithNativeImpl(TSharedPtr<ISentrySpan> spanImpl)
{
	SentrySpanNativeImpl = spanImpl;
}

TSharedPtr<ISentrySpan> USentrySpan::GetNativeImpl()
{
	return SentrySpanNativeImpl;
}
