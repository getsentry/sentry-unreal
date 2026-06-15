// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryId.h"

#if !USE_SENTRY_NATIVE

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryId::FAppleSentryId()
{
	IdApple = [[SENTRY_APPLE_CLASS(SentryObjCId) alloc] init];
}

FAppleSentryId::FAppleSentryId(const FString& id)
{
	IdApple = [[SENTRY_APPLE_CLASS(SentryObjCId) alloc] initWithUUIDString:id.GetNSString()];
}

FAppleSentryId::FAppleSentryId(SentryObjCId* id)
{
	IdApple = id;
}

FAppleSentryId::~FAppleSentryId()
{
	// Put custom destructor logic here if needed
}

SentryObjCId* FAppleSentryId::GetNativeObject()
{
	return IdApple;
}

FString FAppleSentryId::ToString() const
{
	return FString(IdApple.sentryIdString);
}

#endif // !USE_SENTRY_NATIVE
