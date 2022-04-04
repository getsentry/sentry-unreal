// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

@class SentryId;

class SentryIdIOS : public ISentryId
{
public:
	SentryIdIOS();
	SentryIdIOS(SentryId* id);
	virtual ~SentryIdIOS() override;

	SentryId* GetNativeObject();

private:
	SentryId* IdIOS;
};