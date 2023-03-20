// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryAttachmentInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class SentryAttachmentAndroid : public ISentryAttachment, public FSentryJavaClassWrapper
{
public:
	SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType);

	void SetupClassMethods();

	static FName GetClassName();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

private:
	FJavaClassMethod GetDataMethod;
	FJavaClassMethod GetPathMethod;
	FJavaClassMethod GetFilenameMethod;
	FJavaClassMethod GetContentTypeMethod;
};