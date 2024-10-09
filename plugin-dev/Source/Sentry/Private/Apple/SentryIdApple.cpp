// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryIdApple::SentryIdApple()
{
	// `SentryId` definition was moved to Swift so its name that can be recognized by UE should be taken from "Sentry-Swift.h" to successfully load class on Mac
#if PLATFORM_MAC
	IdApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry8SentryId) alloc] init];
#elif PLATFORM_IOS
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
#endif
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
