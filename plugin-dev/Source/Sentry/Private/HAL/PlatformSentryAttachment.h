#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryAttachment.h"
#elif PLATFORM_APPLE
#include "Apple/SentryAttachmentApple.h"
#else
#include "Null/NullSentryAttachment.h"
#endif

static TSharedPtr<ISentryAttachment> CreateSharedSentryAttachment(const FString& Path, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryAttachment(Path, Filename, ContentType));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryAttachmentApple(Path, Filename, ContentType));
#else
	return MakeShareable(new FNullSentryAttachment);
#endif
}

static TSharedPtr<ISentryAttachment> CreateSharedSentryAttachment(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryAttachment(Data, Filename, ContentType));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryAttachmentApple(Data, Filename, ContentType));
#else
	return MakeShareable(new FNullSentryAttachment);
#endif
}
