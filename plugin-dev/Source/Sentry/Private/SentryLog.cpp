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