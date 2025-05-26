// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryAttachment.h"

#include "HAL/PlatformSentryAttachment.h"

void USentryAttachment::InitializeWithData(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType /* = FString(TEXT("application/octet-stream")) */)
{
	NativeImpl = CreateSharedSentryAttachment(Data, Filename, ContentType);
}

void USentryAttachment::InitializeWithPath(const FString& Path, const FString& Filename, const FString& ContentType /* = FString(TEXT("application/octet-stream")) */)
{
	NativeImpl = CreateSharedSentryAttachment(Path, Filename, ContentType);
}

TArray<uint8> USentryAttachment::GetData() const
{
	if (!NativeImpl)
		return TArray<uint8>();

	return NativeImpl->GetData();
}

FString USentryAttachment::GetPath() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetPath();
}

FString USentryAttachment::GetFilename() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetFilename();
}

FString USentryAttachment::GetContentType() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetContentType();
}
