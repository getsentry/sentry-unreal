// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryAttachment
{
public:
	virtual ~ISentryAttachment() = default;

	virtual TArray<uint8> GetData() const = 0;
	virtual FString GetPath() const = 0;
	virtual FString GetFilename() const = 0;
	virtual FString GetContentType() const = 0;
};