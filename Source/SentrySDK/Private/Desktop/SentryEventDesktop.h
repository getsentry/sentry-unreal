// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryEventInterface.h"

class SentryEventDesktop : public ISentryEvent
{
public:
	SentryEventDesktop();
	SentryEventDesktop(sentry_value_t event);
	virtual ~SentryEventDesktop() override;

	sentry_value_t GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual void SetLevel(ESentryLevel level) override;

private:
	sentry_value_t EventDesktop;
};
