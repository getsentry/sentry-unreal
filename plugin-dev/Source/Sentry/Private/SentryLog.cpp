// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLog.h"

#include "HAL/PlatformSentryLog.h"

void USentryLog::Initialize()
{
	NativeImpl = CreateSharedSentryLog();
}

void USentryLog::SetMessage(const FString& InMessage)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(InMessage);
}

FString USentryLog::GetMessage() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
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