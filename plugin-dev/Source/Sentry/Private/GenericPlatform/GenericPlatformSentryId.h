// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryIdInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryId : public ISentryId
{
public:
	FGenericPlatformSentryId();
	FGenericPlatformSentryId(sentry_uuid_t id);
	virtual ~FGenericPlatformSentryId() override;

	sentry_uuid_t GetNativeObject();

	virtual FString ToString() const override;

private:
	sentry_uuid_t Id;
};

#endif
