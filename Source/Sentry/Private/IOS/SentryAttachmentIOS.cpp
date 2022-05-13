// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentIOS.h"

#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryAttachmentIOS::SentryAttachmentIOS(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	AttachmentIOS = [[SentryAttachment alloc] initWithData:SentryConvertorsIOS::ByteDataToNative(data)
		filename:filename.GetNSString() contentType:contentType.GetNSString()];
}

SentryAttachmentIOS::SentryAttachmentIOS(const FString& path, const FString& filename, const FString& contentType)
{
	AttachmentIOS = [[SentryAttachment alloc] initWithPath:path.GetNSString()
		filename:filename.GetNSString() contentType:contentType.GetNSString()];
}

SentryAttachmentIOS::~SentryAttachmentIOS()
{
	// Put custom destructor logic here if needed
}

SentryAttachment* SentryAttachmentIOS::GetNativeObject()
{
	return AttachmentIOS;
}

TArray<uint8> SentryAttachmentIOS::GetData() const
{
	return SentryConvertorsIOS::ByteDataToUnreal(AttachmentIOS.data);
}

FString SentryAttachmentIOS::GetPath() const
{
	return FString(AttachmentIOS.path);
}

FString SentryAttachmentIOS::GetFilename() const
{
	return FString(AttachmentIOS.filename);
}

FString SentryAttachmentIOS::GetContentType() const
{
	return FString(AttachmentIOS.contentType);
}
