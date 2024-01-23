﻿// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentrySamplingContext
{
public:
	virtual ~ISentrySamplingContext() = default;

	virtual TMap<FString, FString> GetCustomSamplingContext() const = 0;
};