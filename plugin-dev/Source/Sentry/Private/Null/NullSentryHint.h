// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryHintInterface.h"

class ISentryAttachment;

class FNullSentryHint final : public ISentryHint
{
public:
	virtual ~FNullSentryHint() override = default;

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual int32 RemoveAttachments(const FString& filenamePattern) override { return 0; }
	virtual void ClearAttachments() override {}
};

typedef FNullSentryHint FPlatformSentryHint;
