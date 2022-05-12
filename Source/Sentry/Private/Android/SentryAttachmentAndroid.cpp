// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
{
	
}

SentryAttachmentAndroid::~SentryAttachmentAndroid()
{
	
}

jobject SentryAttachmentAndroid::GetNativeObject()
{
	return AttachmentAndroid;
}

TArray<uint8> SentryAttachmentAndroid::GetData() const
{
	return TArray<uint8>();
}

FString SentryAttachmentAndroid::GetPath() const
{
	return FString();
}

FString SentryAttachmentAndroid::GetFilename() const
{
	return FString();
}

FString SentryAttachmentAndroid::GetContentType() const
{
	return FString();
}