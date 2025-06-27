// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryAttachmentInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryAttachment : public ISentryAttachment
{
public:
	FGenericPlatformSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType);
	FGenericPlatformSentryAttachment(const FString& path, const FString& filename, const FString& contentType);
	virtual ~FGenericPlatformSentryAttachment() override;

	void SetNativeObject(sentry_attachment_t* attachment);
	sentry_attachment_t* GetNativeObject();

	virtual TArray<uint8> GetData() const override;
	virtual FString GetPath() const override;
	virtual FString GetFilename() const override;
	virtual FString GetContentType() const override;

	const TArray<uint8>& GetDataByRef() const;

private:
	TArray<uint8> Data;
	FString Path;
	FString Filename;
	FString ContentType;

	sentry_attachment_t* Attachment;
};

typedef FGenericPlatformSentryAttachment FPlatformSentryAttachment;

#endif
