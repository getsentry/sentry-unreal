// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryId::FAppleSentryId()
{
	// `SentryId` definition was moved to Swift so its name that can be recognized by UE should be taken from "Sentry-Swift.h" to successfully load class on Mac
#if PLATFORM_MAC
	IdApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry8SentryId) alloc] init];
#elif PLATFORM_IOS
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
#endif
}

FAppleSentryId::FAppleSentryId(const FString& id)
{
	// `SentryId` definition was moved to Swift so its name that can be recognized by UE should be taken from "Sentry-Swift.h" to successfully load class on Mac
#if PLATFORM_MAC
	IdApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry8SentryId) alloc] initWithUUIDString:id.GetNSString()];
#elif PLATFORM_IOS
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] initWithUUIDString:id.GetNSString()];
#endif
}

FAppleSentryId::FAppleSentryId(SentryId* id)
{
	IdApple = id;
}

FAppleSentryId::~FAppleSentryId()
{
	// Put custom destructor logic here if needed
}

SentryId* FAppleSentryId::GetNativeObject()
{
	return IdApple;
}

FString FAppleSentryId::ToString() const
{
	return FString(IdApple.sentryIdString);
}
