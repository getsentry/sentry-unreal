#pragma once

#if USE_SENTRY_NATIVE

class FWindowsSentryConverters
{
	static void SentryCrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize);
};

#endif // USE_SENTRY_NATIVE
