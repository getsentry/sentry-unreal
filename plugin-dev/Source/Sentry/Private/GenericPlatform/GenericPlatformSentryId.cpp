// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryId.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryId::FGenericPlatformSentryId()
{
	Id = sentry_uuid_new_v4();
}

FGenericPlatformSentryId::FGenericPlatformSentryId(const FString& id)
{
	Id = sentry_uuid_from_string(TCHAR_TO_UTF8(*id));
}

FGenericPlatformSentryId::FGenericPlatformSentryId(sentry_uuid_t id)
{
	Id = id;
}

FGenericPlatformSentryId::~FGenericPlatformSentryId()
{
	// Put custom destructor logic here if needed
}

sentry_uuid_t FGenericPlatformSentryId::GetNativeObject()
{
	return Id;
}

FString FGenericPlatformSentryId::ToString() const
{
	char IdString[37];
	sentry_uuid_as_string(&Id, IdString);
	return FString(IdString);
}

#endif
