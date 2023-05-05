// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryAttachmentInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryAttachmentAndroid : public ISentryAttachment, public FSentryJavaObjectWrapper
{
public:
	SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType);

	void SetupClassMethods();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

private:
	FSentryJavaMethod GetDataMethod;
	FSentryJavaMethod GetPathMethod;
	FSentryJavaMethod GetFilenameMethod;
	FSentryJavaMethod GetContentTypeMethod;
};