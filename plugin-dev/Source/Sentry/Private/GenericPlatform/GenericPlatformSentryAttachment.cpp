// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryAttachment.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryAttachment::FGenericPlatformSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: Data(data), Filename(filename), ContentType(contentType), Uuid(sentry_uuid_nil())
{
}

FGenericPlatformSentryAttachment::FGenericPlatformSentryAttachment(const FString& path, const FString& filename, const FString& contentType)
	: Path(path), Filename(filename), ContentType(contentType), Uuid(sentry_uuid_nil())
{
}

FGenericPlatformSentryAttachment::~FGenericPlatformSentryAttachment()
{
	// Put custom destructor logic here if needed
}

void FGenericPlatformSentryAttachment::SetUuid(sentry_uuid_t uuid)
{
	Uuid = uuid;
}

sentry_uuid_t FGenericPlatformSentryAttachment::GetUuid()
{
	return Uuid;
}

TArray<uint8> FGenericPlatformSentryAttachment::GetData() const
{
	return Data;
}

FString FGenericPlatformSentryAttachment::GetPath() const
{
	return Path;
}

FString FGenericPlatformSentryAttachment::GetFilename() const
{
	return Filename;
}

FString FGenericPlatformSentryAttachment::GetContentType() const
{
	return ContentType;
}

const TArray<uint8>& FGenericPlatformSentryAttachment::GetDataByRef() const
{
	return Data;
}

#endif
