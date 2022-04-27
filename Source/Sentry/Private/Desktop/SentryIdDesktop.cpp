// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdDesktop.h"

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
