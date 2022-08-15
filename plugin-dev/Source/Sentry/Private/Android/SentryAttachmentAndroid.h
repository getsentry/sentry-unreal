// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

#include "Interface/SentryAttachmentInterface.h"

class SentryAttachmentAndroid : public ISentryAttachment
{
public:
	SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType);
	virtual ~SentryAttachmentAndroid() override;

	jobject GetNativeObject();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

private:
	jobject AttachmentAndroid;
};