// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachment.h"

#include "Interface/SentryAttachmentInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryAttachmentAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryAttachmentApple.h"
#endif

void USentryAttachment::InitializeWithData(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
	AttachmentNativeImpl = MakeShareable(new SentryAttachmentAndroid(Data, Filename, ContentType));
#elif PLATFORM_IOS || PLATFORM_MAC
	AttachmentNativeImpl = MakeShareable(new SentryAttachmentApple(Data, Filename, ContentType));
#endif
}

void USentryAttachment::InitializeWithPath(const FString& Path, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
	AttachmentNativeImpl = MakeShareable(new SentryAttachmentAndroid(Path, Filename, ContentType));
#elif PLATFORM_IOS || PLATFORM_MAC
	AttachmentNativeImpl = MakeShareable(new SentryAttachmentApple(Path, Filename, ContentType));
#endif
}

TArray<uint8> USentryAttachment::GetData() const
{
	if(!AttachmentNativeImpl)
		return TArray<uint8>();

	return AttachmentNativeImpl->GetData();
}

FString USentryAttachment::GetPath() const
{
	if(!AttachmentNativeImpl)
		return FString();

	return AttachmentNativeImpl->GetPath();
}

FString USentryAttachment::GetFilename() const
{
	if(!AttachmentNativeImpl)
		return FString();

	return AttachmentNativeImpl->GetFilename();
}

FString USentryAttachment::GetContentType() const
{
	if(!AttachmentNativeImpl)
		return FString();

	return AttachmentNativeImpl->GetContentType();
}

void USentryAttachment::InitWithNativeImpl(TSharedPtr<ISentryAttachment> attachmentImpl)
{
	AttachmentNativeImpl = attachmentImpl;
}

TSharedPtr<ISentryAttachment> USentryAttachment::GetNativeImpl()
{
	return AttachmentNativeImpl;
}
