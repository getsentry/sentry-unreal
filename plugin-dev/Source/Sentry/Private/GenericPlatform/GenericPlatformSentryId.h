// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryIdInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryId : public ISentryId
{
public:
	FGenericPlatformSentryId();
	FGenericPlatformSentryId(const FString& id);
	FGenericPlatformSentryId(sentry_uuid_t id);
	virtual ~FGenericPlatformSentryId() override;

	sentry_uuid_t GetNativeObject();

	virtual FString ToString() const override;

private:
	sentry_uuid_t Id;
};

typedef FGenericPlatformSentryId FPlatformSentryId;

#endif
