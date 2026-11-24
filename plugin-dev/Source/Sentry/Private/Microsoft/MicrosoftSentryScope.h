// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentryScope.h"

class FMicrosoftSentryScope : public FGenericPlatformSentryScope
{
public:
	virtual ~FMicrosoftSentryScope() override = default;

protected:
	virtual void AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment, sentry_scope_t* scope) override;
	virtual void AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment, sentry_scope_t* scope) override;
};

typedef FMicrosoftSentryScope FPlatformSentryScope;

#endif
