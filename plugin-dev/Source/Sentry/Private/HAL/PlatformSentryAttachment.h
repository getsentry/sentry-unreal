#pragma once

#include "Interface/SentryAttachmentInterface.h"

static ISentryAttachment* CreateSentryAttachment(const FString& Path, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
#include "Android/SentryAttachmentAndroid.h"
	return new SentryAttachmentAndroid(Path, Filename, ContentType);
#elif PLATFORM_APPLE
#include "Apple/SentryAttachmentApple.h"
	return new SentryAttachmentApple(Path, Filename, ContentType);
#else
#include "Null/NullSentryAttachment.h"
	return new FNullSentryAttachment();
#endif
}

static ISentryAttachment* CreateSentryAttachment(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
#if PLATFORM_ANDROID
#include "Android/SentryAttachmentAndroid.h"
	return new SentryAttachmentAndroid(Data, Filename, ContentType);
#elif PLATFORM_APPLE
#include "Apple/SentryAttachmentApple.h"
	return new SentryAttachmentApple(Data, Filename, ContentType);
#else
#include "Null/NullSentryAttachment.h"
	return new FNullSentryAttachment();
#endif
}
