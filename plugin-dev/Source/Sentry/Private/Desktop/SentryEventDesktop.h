// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryEventInterface.h"

#if USE_SENTRY_NATIVE

class SentryEventDesktop : public ISentryEvent
{
public:
	SentryEventDesktop();
	SentryEventDesktop(sentry_value_t event);
	virtual ~SentryEventDesktop() override;

	sentry_value_t GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual bool IsCrash() const override;

private:
	sentry_value_t EventDesktop;
};

#endif
