// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryIdApple::SentryIdApple()
{
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
}

SentryIdApple::SentryIdApple(SentryId* id)
{
	IdApple = id;
}

SentryIdApple::~SentryIdApple()
{
	// Put custom destructor logic here if needed
}

SentryId* SentryIdApple::GetNativeObject()
{
	return IdApple;
}

FString SentryIdApple::ToString() const
{
	return FString(IdApple.sentryIdString);
}
