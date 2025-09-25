// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryLog.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryLog::FAppleSentryLog()
{
	LogApple = [[SENTRY_APPLE_CLASS(SentryLog) alloc] init];
}

FAppleSentryLog::FAppleSentryLog(SentryLog* log)
{
	LogApple = log;
}

FAppleSentryLog::FAppleSentryLog(const FString& message, ESentryLevel level, const FString& category)
{
	LogApple = [[SENTRY_APPLE_CLASS(SentryLog) alloc] init];
	SetMessage(message);
	SetLevel(level);
}

FAppleSentryLog::~FAppleSentryLog()
{
	// Put custom destructor logic here if needed
}

SentryLog* FAppleSentryLog::GetNativeObject()
{
	return LogApple;
}

void FAppleSentryLog::SetMessage(const FString& message)
{
	LogApple.message = message.GetNSString();
}

FString FAppleSentryLog::GetMessage() const
{
	return FString(LogApple.message);
}

void FAppleSentryLog::SetLevel(ESentryLevel level)
{
	LogApple.level = FAppleSentryConverters::SentryLevelToNative(level);
}

ESentryLevel FAppleSentryLog::GetLevel() const
{
	return FAppleSentryConverters::SentryLevelToUnreal(LogApple.level);
}