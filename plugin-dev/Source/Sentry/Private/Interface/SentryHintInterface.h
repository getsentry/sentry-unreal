// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryAttachment;

class ISentryHint
{
public:
	virtual ~ISentryHint() = default;

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) = 0;
};