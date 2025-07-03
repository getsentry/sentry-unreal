// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryAttachment.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryAttachment.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryAttachment.h"
#else
#include "Null/NullSentryAttachment.h"
#endif

static TSharedPtr<ISentryAttachment> CreateSharedSentryAttachment(const FString& Path, const FString& Filename, const FString& ContentType)
{
	return MakeShareable(new FPlatformSentryAttachment(Path, Filename, ContentType));
}

static TSharedPtr<ISentryAttachment> CreateSharedSentryAttachment(const TArray<uint8>& Data, const FString& Filename, const FString& ContentType)
{
	return MakeShareable(new FPlatformSentryAttachment(Data, Filename, ContentType));
}
