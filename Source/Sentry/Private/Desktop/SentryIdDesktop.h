// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryIdInterface.h"

class SentryIdDesktop : public ISentryId
{
public:
	SentryIdDesktop();
	SentryIdDesktop(sentry_uuid_t id);
	virtual ~SentryIdDesktop() override;

	sentry_uuid_t GetNativeObject();

private:
	sentry_uuid_t IdDesktop;
};