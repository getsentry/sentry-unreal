// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryLog.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryLog::FAppleSentryLog()
{
	LogApple = [SENTRY_APPLE_CLASS(SentryLog) alloc];
	// Initialize required properties
	LogApple.timestamp = [NSDate date];
	LogApple.traceId = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
	LogApple.body = @"";
	LogApple.attributes = @{};
	LogApple.level = SentryStructuredLogLevelDebug;
}

FAppleSentryLog::FAppleSentryLog(SentryLog* log)
{
	LogApple = log;
}

FAppleSentryLog::FAppleSentryLog(const FString& body, ESentryLevel level)
{
	LogApple = [SENTRY_APPLE_CLASS(SentryLog) alloc];
	// Initialize required properties
	LogApple.timestamp = [NSDate date];
	LogApple.traceId = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
	LogApple.attributes = @{};
	SetBody(body);
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

void FAppleSentryLog::SetBody(const FString& body)
{
	LogApple.body = body.GetNSString();
}

FString FAppleSentryLog::GetBody() const
{
	return FString(LogApple.body);
}

void FAppleSentryLog::SetLevel(ESentryLevel level)
{
	LogApple.level = FAppleSentryConverters::SentryStructuredLogLevelToNative(level);
}

ESentryLevel FAppleSentryLog::GetLevel() const
{
	return FAppleSentryConverters::SentryStructuredLogLevelToUnreal(LogApple.level);
}