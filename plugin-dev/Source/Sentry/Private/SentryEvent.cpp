// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryEvent.h"

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

void USentryEvent::SetTag(const FString& Key, const FString& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetTag(Key, Value);
}

FString USentryEvent::GetTag(const FString& Key) const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetTag(Key);
}

bool USentryEvent::TryGetTag(const FString& Key, FString& Value) const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->TryGetTag(Key, Value);
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

void USentryEvent::SetContext(const FString& Key, const TMap<FString, FSentryVariant>& Values)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetContext(Key, Values);
}

TMap<FString, FSentryVariant> USentryEvent::GetContext(const FString& Key) const
{
	if (!NativeImpl)
		return TMap<FString, FSentryVariant>();

	return NativeImpl->GetContext(Key);
}

bool USentryEvent::TryGetContext(const FString& Key, TMap<FString, FSentryVariant>& Value) const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->TryGetContext(Key, Value);
}

void USentryEvent::RemoveContext(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveContext(Key);
}

void USentryEvent::SetExtra(const FString& Key, const FSentryVariant& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtra(Key, Value);
}

FSentryVariant USentryEvent::GetExtra(const FString& Key) const
{
	if (!NativeImpl)
		return FSentryVariant();

	return NativeImpl->GetExtra(Key);
}

bool USentryEvent::TryGetExtra(const FString& Key, FSentryVariant& Value) const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->TryGetExtra(Key, Value);
}

void USentryEvent::RemoveExtra(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveExtra(Key);
}

void USentryEvent::SetExtras(const TMap<FString, FSentryVariant>& Extras)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtras(Extras);
}

TMap<FString, FSentryVariant> USentryEvent::GetExtras() const
{
	if (!NativeImpl)
		return TMap<FString, FSentryVariant>();

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
