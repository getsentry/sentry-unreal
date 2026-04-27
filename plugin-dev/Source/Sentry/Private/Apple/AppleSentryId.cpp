// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryId.h"

#if !USE_SENTRY_NATIVE

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryId::FAppleSentryId()
{
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] init];
}

FAppleSentryId::FAppleSentryId(const FString& id)
{
	IdApple = [[SENTRY_APPLE_CLASS(SentryId) alloc] initWithUUIDString:id.GetNSString()];
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

#endif // !USE_SENTRY_NATIVE
