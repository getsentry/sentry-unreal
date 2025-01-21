// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachment.h"

#include "Interface/SentryAttachmentInterface.h"

TArray<uint8> USentryAttachment::GetData() const
{
	if(!NativeImpl)
		return TArray<uint8>();

	return NativeImpl->GetData();
}

FString USentryAttachment::GetPath() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetPath();
}

FString USentryAttachment::GetFilename() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetFilename();
}

FString USentryAttachment::GetContentType() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetContentType();
}
