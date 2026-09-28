// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ISentryAttachment;

class ISentryHint
{
public:
	virtual ~ISentryHint() = default;

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) = 0;
	virtual int32 RemoveAttachments(const FString& filenamePattern) = 0;
	virtual void ClearAttachments() = 0;
};