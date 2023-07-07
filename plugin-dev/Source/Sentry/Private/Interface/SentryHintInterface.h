// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USentryAttachment;

class ISentryHint
{
public:
	virtual ~ISentryHint() = default;

	virtual void AddAttachment(USentryAttachment* attachment) = 0;
};