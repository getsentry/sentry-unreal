#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryTransactionContextAndroid.h"
#define NEW_SENTRY_TRANSACTION_CONTEXT(Name, Operation) new SentryTransactionContextAndroid(Name, Operation)
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionContextApple.h"
#define NEW_SENTRY_TRANSACTION_CONTEXT(Name, Operation) new SentryTransactionContextApple(Name, Operation)
#else
#include "GenericPlatform/GenericPlatformSentryTransactionContext.h"
#define NEW_SENTRY_TRANSACTION_CONTEXT(Name, Operation) new FGenericPlatformSentryTransactionContext(Name, Operation)
#endif

#ifndef NEW_SENTRY_TRANSACTION_CONTEXT
#error Make sure the NEW_SENTRY_TRANSACTION_CONTEXT macro is defined for supported platforms
#endif
