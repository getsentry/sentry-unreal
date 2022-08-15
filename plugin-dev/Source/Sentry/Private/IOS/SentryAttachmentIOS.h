// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryAttachmentInterface.h"

@class SentryAttachment;

class SentryAttachmentIOS : public ISentryAttachment
{
public:
	SentryAttachmentIOS(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	SentryAttachmentIOS(const FString& path, const FString& filename, const FString& contentType);
	virtual ~SentryAttachmentIOS() override;

	SentryAttachment* GetNativeObject();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

private:
	SentryAttachment* AttachmentIOS;
};
