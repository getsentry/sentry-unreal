// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLogData.h"

#include "HAL/PlatformSentryLog.h"

void USentryLogData::Initialize()
{
	NativeImpl = CreateSharedSentryLog();
}

void USentryLogData::SetMessage(const FString& InMessage)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(InMessage);
}

FString USentryLogData::GetMessage() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
}

void USentryLogData::SetLevel(ESentryLevel InLevel)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(InLevel);
}

ESentryLevel USentryLogData::GetLevel() const
{
	if (!NativeImpl)
		return ESentryLevel::Info;

	return NativeImpl->GetLevel();
}