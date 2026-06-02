// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryLog.h"

#if !USE_SENTRY_NATIVE

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryLog::FAppleSentryLog()
	: FAppleSentryLog(FString(), ESentryLevel::Debug)
{
}

FAppleSentryLog::FAppleSentryLog(SentryObjCLog* log)
{
	LogApple = log;
}

FAppleSentryLog::FAppleSentryLog(const FString& body, ESentryLevel level)
{
	LogApple = [[SENTRY_APPLE_CLASS(SentryObjCLog) alloc] initWithLevel:FAppleSentryConverters::SentryLogLevelToNative(level)
																   body:body.GetNSString()];
}

FAppleSentryLog::~FAppleSentryLog()
{
	// Put custom destructor logic here if needed
}

SentryObjCLog* FAppleSentryLog::GetNativeObject()
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
	LogApple.level = FAppleSentryConverters::SentryLogLevelToNative(level);
}

ESentryLevel FAppleSentryLog::GetLevel() const
{
	return FAppleSentryConverters::SentryLogLevelToUnreal(LogApple.level);
}

void FAppleSentryLog::SetAttribute(const FString& key, const FSentryVariant& value)
{
	SentryObjCAttribute* attribute = FAppleSentryConverters::VariantToAttributeNative(value);

	if (!attribute)
	{
		return;
	}

	[LogApple setAttribute:attribute forKey:key.GetNSString()];
}

FSentryVariant FAppleSentryLog::GetAttribute(const FString& key) const
{
	SentryObjCAttribute* attribute = [LogApple.attributes objectForKey:key.GetNSString()];

	if (!attribute)
	{
		return FSentryVariant();
	}

	return FAppleSentryConverters::SentryAttributeToVariant(attribute);
}

bool FAppleSentryLog::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	SentryObjCAttribute* attribute = [LogApple.attributes objectForKey:key.GetNSString()];

	if (!attribute)
	{
		return false;
	}

	value = FAppleSentryConverters::SentryAttributeToVariant(attribute);
	return true;
}

void FAppleSentryLog::RemoveAttribute(const FString& key)
{
	[LogApple setAttribute:nil forKey:key.GetNSString()];
}

void FAppleSentryLog::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	for (const auto& pair : attributes)
	{
		SetAttribute(pair.Key, pair.Value);
	}
}

#endif // !USE_SENTRY_NATIVE