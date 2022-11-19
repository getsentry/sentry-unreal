// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryId
{
public:
	virtual ~ISentryId() = default;

	virtual FString ToString() const = 0;
};