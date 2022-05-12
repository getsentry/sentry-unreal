// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentIOS.h"

SentryAttachmentIOS::SentryAttachmentIOS(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	
}

SentryAttachmentIOS::SentryAttachmentIOS(const FString& path, const FString& filename, const FString& contentType)
{
	
}

SentryAttachmentIOS::~SentryAttachmentIOS()
{
	
}

SentryAttachment* SentryAttachmentIOS::GetNativeObject()
{
	return AttachmentIOS;
}

TArray<uint8> SentryAttachmentIOS::GetData() const
{
	return TArray<uint8>();
}

FString SentryAttachmentIOS::GetPath() const
{
	return FString();
}

FString SentryAttachmentIOS::GetFilename() const
{
	return FString();
}

FString SentryAttachmentIOS::GetContentType() const
{
	return FString();
}
