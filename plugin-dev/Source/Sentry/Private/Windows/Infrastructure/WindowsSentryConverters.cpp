// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentryConverters.h"

#if USE_SENTRY_NATIVE

/* static */ void FWindowsSentryConverters::SentryCrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize)
{
	EXCEPTION_RECORD* ExceptionRecord = crashContext->exception_ptrs.ExceptionRecord;

	FString ErrorString = TEXT("Unhandled Exception: ");

#define HANDLE_CASE(x)           \
	case x:                      \
		ErrorString += TEXT(#x); \
		break;

	switch (ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		ErrorString += TEXT("EXCEPTION_ACCESS_VIOLATION ");
		if (ExceptionRecord->ExceptionInformation[0] == 0)
		{
			ErrorString += TEXT("reading address ");
		}
		else if (ExceptionRecord->ExceptionInformation[0] == 1)
		{
			ErrorString += TEXT("writing address ");
		}
		ErrorString += FString::Printf(
#if PLATFORM_64BITS
			TEXT("0x%016llx"),
#else
			TEXT("0x%08x"),
#endif
			ExceptionRecord->ExceptionInformation[1]);
		break;
		HANDLE_CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
		HANDLE_CASE(EXCEPTION_DATATYPE_MISALIGNMENT)
		HANDLE_CASE(EXCEPTION_FLT_DENORMAL_OPERAND)
		HANDLE_CASE(EXCEPTION_FLT_DIVIDE_BY_ZERO)
		HANDLE_CASE(EXCEPTION_FLT_INVALID_OPERATION)
		HANDLE_CASE(EXCEPTION_ILLEGAL_INSTRUCTION)
		HANDLE_CASE(EXCEPTION_INT_DIVIDE_BY_ZERO)
		HANDLE_CASE(EXCEPTION_PRIV_INSTRUCTION)
		HANDLE_CASE(EXCEPTION_STACK_OVERFLOW)
	default:
		ErrorString += FString::Printf(TEXT("0x%08x"), (uint32)ExceptionRecord->ExceptionCode);
	}

	FCString::Strncpy(outErrorString, *ErrorString, errorStringBufSize);

#undef HANDLE_CASE
}

#endif // USE_SENTRY_NATIVE
