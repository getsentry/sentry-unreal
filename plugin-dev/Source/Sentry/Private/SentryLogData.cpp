// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLogData.h"

void USentryLogData::Initialize()
{
	Message = FString();
	Level = ESentryLevel::Info;
}

void USentryLogData::SetMessage(const FString& InMessage)
{
	Message = InMessage;
}

FString USentryLogData::GetMessage() const
{
	return Message;
}

void USentryLogData::SetLevel(ESentryLevel InLevel)
{
	Level = InLevel;
}

ESentryLevel USentryLogData::GetLevel() const
{
	return Level;
}