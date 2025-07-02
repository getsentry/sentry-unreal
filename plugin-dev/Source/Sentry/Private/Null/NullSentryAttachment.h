// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryAttachmentInterface.h"

class FNullSentryAttachment final : public ISentryAttachment
{
public:
	FNullSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType) {}
	FNullSentryAttachment(const FString& path, const FString& filename, const FString& contentType) {}

	virtual ~FNullSentryAttachment() override = default;

	virtual TArray<uint8> GetData() const override { return {}; }
	virtual FString GetPath() const override { return TEXT(""); }
	virtual FString GetFilename() const override { return TEXT(""); }
	virtual FString GetContentType() const override { return TEXT(""); }
};

typedef FNullSentryAttachment FPlatformSentryAttachment;