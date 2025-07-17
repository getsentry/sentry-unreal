// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryAttachment.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryAttachment::FAppleSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	AttachmentApple = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithData:FAppleSentryConverters::ByteDataToNative(data)
																		filename:filename.GetNSString()
																	 contentType:contentType.GetNSString()];
}

FAppleSentryAttachment::FAppleSentryAttachment(const FString& path, const FString& filename, const FString& contentType)
{
	AttachmentApple = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithPath:path.GetNSString()
																		filename:filename.GetNSString()
																	 contentType:contentType.GetNSString()];
}

FAppleSentryAttachment::~FAppleSentryAttachment()
{
	// Put custom destructor logic here if needed
}

SentryAttachment* FAppleSentryAttachment::GetNativeObject()
{
	return AttachmentApple;
}

TArray<uint8> FAppleSentryAttachment::GetData() const
{
	return FAppleSentryConverters::ByteDataToUnreal(AttachmentApple.data);
}

FString FAppleSentryAttachment::GetPath() const
{
	return FString(AttachmentApple.path);
}

FString FAppleSentryAttachment::GetFilename() const
{
	return FString(AttachmentApple.filename);
}

FString FAppleSentryAttachment::GetContentType() const
{
	return FString(AttachmentApple.contentType);
}
