// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryAttachmentInterface.h"

@class SentryAttachment;

class FAppleSentryAttachment : public ISentryAttachment
{
public:
	FAppleSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	FAppleSentryAttachment(const FString& path, const FString& filename, const FString& contentType);
	virtual ~FAppleSentryAttachment() override;

	SentryAttachment* GetNativeObject();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

private:
	SentryAttachment* AttachmentApple;
};

typedef FAppleSentryAttachment FPlatformSentryAttachment;
