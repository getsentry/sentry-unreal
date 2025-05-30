// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryEvent.h"

#include "Algo/Find.h"
#include "HAL/PlatformSentryEvent.h"
#include "Interface/SentryIdInterface.h"

void USentryEvent::Initialize()
{
	NativeImpl = CreateSharedSentryEvent();
}

FString USentryEvent::GetId() const
{
	if (!NativeImpl)
		return FString();

	TSharedPtr<ISentryId> idNativeImpl = NativeImpl->GetId();
	if (!idNativeImpl)
		return FString();

	return idNativeImpl->ToString();
}

void USentryEvent::SetMessage(const FString& Message)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(Message);
}

FString USentryEvent::GetMessage() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
}

void USentryEvent::SetLevel(ESentryLevel Level)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(Level);
}

ESentryLevel USentryEvent::GetLevel() const
{
	if (!NativeImpl)
		return ESentryLevel::Debug;

	return NativeImpl->GetLevel();
}

void USentryEvent::SetFingerprint(const TArray<FString>& Fingerprint)
{
	if (!NativeImpl)
		return;

	return NativeImpl->SetFingerprint(Fingerprint);
}

TArray<FString> USentryEvent::GetFingerprint() const
{
	if (!NativeImpl)
		return TArray<FString>();

	return NativeImpl->GetFingerprint();
}

void USentryEvent::SetTagValue(const FString& Key, const FString& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetTagValue(Key, Value);
}

FString USentryEvent::GetTagValue(const FString& Key) const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetTagValue(Key);
}

void USentryEvent::RemoveTag(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveTag(Key);
}

void USentryEvent::SetTags(const TMap<FString, FString>& Tags)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetTags(Tags);
}

TMap<FString, FString> USentryEvent::GetTags() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetTags();
}

void USentryEvent::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetContext(Key, Values);
}

void USentryEvent::RemoveContext(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveContext(Key);
}

void USentryEvent::SetExtraValue(const FString& Key, const FString& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtraValue(Key, Value);
}

FString USentryEvent::GetExtraValue(const FString& Key) const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetExtraValue(Key);
}

void USentryEvent::RemoveExtra(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveExtra(Key);
}

void USentryEvent::SetExtras(const TMap<FString, FString>& Extras)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtras(Extras);
}

TMap<FString, FString> USentryEvent::GetExtras() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetExtras();
}

bool USentryEvent::IsCrash() const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->IsCrash();
}

bool USentryEvent::IsAnr() const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->IsAnr();
}
