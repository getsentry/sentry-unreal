#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryAttachmentAndroid.h"
#define NEW_SENTRY_ATTACHMENT(Data, Filename, ContentType) new SentryAttachmentAndroid(Data, Filename, ContentType)
#elif PLATFORM_APPLE
#include "Apple/SentryAttachmentApple.h"
#define NEW_SENTRY_ATTACHMENT(Data, Filename, ContentType) new SentryAttachmentApple(Data, Filename, ContentType)
#else
#include "Null/NullSentryAttachment.h"
#define NEW_SENTRY_ATTACHMENT(Data, Filename, ContentType) new FNullSentryAttachment()
#endif

#ifndef NEW_SENTRY_ATTACHMENT
#error Make sure the NEW_SENTRY_ATTACHMENT macro is defined for supported platforms
#endif
