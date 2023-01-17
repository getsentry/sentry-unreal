// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryAttachmentApple::SentryAttachmentApple(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	AttachmentApple = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithData:SentryConvertorsApple::ByteDataToNative(data)
		filename:filename.GetNSString() contentType:contentType.GetNSString()];
}

SentryAttachmentApple::SentryAttachmentApple(const FString& path, const FString& filename, const FString& contentType)
{
	AttachmentApple = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithPath:path.GetNSString()
		filename:filename.GetNSString() contentType:contentType.GetNSString()];
}

SentryAttachmentApple::~SentryAttachmentApple()
{
	// Put custom destructor logic here if needed
}

SentryAttachment* SentryAttachmentApple::GetNativeObject()
{
	return AttachmentApple;
}

TArray<uint8> SentryAttachmentApple::GetData() const
{
	return SentryConvertorsApple::ByteDataToUnreal(AttachmentApple.data);
}

FString SentryAttachmentApple::GetPath() const
{
	return FString(AttachmentApple.path);
}

FString SentryAttachmentApple::GetFilename() const
{
	return FString(AttachmentApple.filename);
}

FString SentryAttachmentApple::GetContentType() const
{
	return FString(AttachmentApple.contentType);
}
