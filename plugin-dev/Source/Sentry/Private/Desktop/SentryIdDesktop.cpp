// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdDesktop.h"

#if USE_SENTRY_NATIVE

SentryIdDesktop::SentryIdDesktop()
{
	IdDesktop = sentry_uuid_new_v4();
}

SentryIdDesktop::SentryIdDesktop(sentry_uuid_t id)
{
	IdDesktop = id;
}

SentryIdDesktop::~SentryIdDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_uuid_t SentryIdDesktop::GetNativeObject()
{
	return IdDesktop;
}

FString SentryIdDesktop::ToString() const
{
	char IdString[37];
	sentry_uuid_as_string(&IdDesktop, IdString);
	return FString(IdString);
}

#endif
