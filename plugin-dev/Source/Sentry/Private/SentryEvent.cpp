// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "SentryId.h"

#include "HAL/PlatformSentryEvent.h"

void USentryEvent::Initialize(const FString& Message, ESentryLevel Level)
{
	NativeImpl = CreateSharedSentryEvent();

	SetMessage(Message);
	SetLevel(Level);
}

USentryId* USentryEvent::GetId() const
{
	if (!NativeImpl)
		return nullptr;

	TSharedPtr<ISentryId> idNativeImpl = NativeImpl->GetId();

	return USentryId::Create(idNativeImpl);
}

void USentryEvent::SetMessage(const FString &Message)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(Message);
}

FString USentryEvent::GetMessage() const
{
	if(!NativeImpl)
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
	if(!NativeImpl)
		return ESentryLevel::Debug;

	return NativeImpl->GetLevel();
}

bool USentryEvent::IsCrash() const
{
	if(!NativeImpl)
		return false;

	return NativeImpl->IsCrash();
}

bool USentryEvent::IsAnr() const
{
	if(!NativeImpl)
		return false;

	return NativeImpl->IsAnr();
}
