// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLog.h"

#include "HAL/PlatformSentryLog.h"

void USentryLog::Initialize()
{
	NativeImpl = CreateSharedSentryLog();
}

void USentryLog::SetBody(const FString& InBody)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetBody(InBody);
}

FString USentryLog::GetBody() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetBody();
}

void USentryLog::SetLevel(ESentryLevel InLevel)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(InLevel);
}

ESentryLevel USentryLog::GetLevel() const
{
	if (!NativeImpl)
		return ESentryLevel::Info;

	return NativeImpl->GetLevel();
}

void USentryLog::SetAttribute(const FString& Key, const FSentryVariant& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetAttribute(Key, Value);
}

FSentryVariant USentryLog::GetAttribute(const FString& Key) const
{
	if (!NativeImpl)
		return FSentryVariant();

	return NativeImpl->GetAttribute(Key);
}

bool USentryLog::TryGetAttribute(const FString& Key, FSentryVariant& Value) const
{
	if (!NativeImpl)
		return false;

	return NativeImpl->TryGetAttribute(Key, Value);
}

void USentryLog::RemoveAttribute(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveAttribute(Key);
}

void USentryLog::AddAttributes(const TMap<FString, FSentryVariant>& Attributes)
{
	if (!NativeImpl)
		return;

	NativeImpl->AddAttributes(Attributes);
}