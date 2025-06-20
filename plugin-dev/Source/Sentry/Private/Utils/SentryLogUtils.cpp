// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLogUtils.h"

#include "CoreGlobals.h"
#include "GenericPlatform/GenericPlatformStackWalk.h"
#include "HAL/UnrealMemory.h"
#include "Misc/OutputDeviceRedirector.h"

void SentryLogUtils::LogStackTrace(const TCHAR* Heading, const ELogVerbosity::Type LogVerbosity, int FramesToSkip)
{
#if !NO_LOGGING
	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)FMemory::SystemMalloc(StackTraceSize);

	{
		StackTrace[0] = 0;
		FGenericPlatformStackWalk::StackWalkAndDumpEx(StackTrace, StackTraceSize, FramesToSkip + 1, FGenericPlatformStackWalk::EStackWalkFlags::AccurateStackWalk);
	}

	FDebug::LogFormattedMessageWithCallstack(LogOutputDevice.GetCategoryName(), __FILE__, __LINE__, Heading, ANSI_TO_TCHAR(StackTrace), LogVerbosity);

	GLog->Flush();

	FMemory::SystemFree(StackTrace);
#endif
}

ESentryLevel SentryLogUtils::ConvertLogVerbosityToSentryLevel(const ELogVerbosity::Type LogVerbosity)
{
	switch (LogVerbosity)
	{
	case ELogVerbosity::Fatal:
		return ESentryLevel::Fatal;
	case ELogVerbosity::Error:
		return ESentryLevel::Error;
	case ELogVerbosity::Warning:
		return ESentryLevel::Warning;
	case ELogVerbosity::Display:
	case ELogVerbosity::Log:
		return ESentryLevel::Info;
	case ELogVerbosity::Verbose:
	case ELogVerbosity::VeryVerbose:
		return ESentryLevel::Debug;
	default:
		return ESentryLevel::Debug;
	}
}
