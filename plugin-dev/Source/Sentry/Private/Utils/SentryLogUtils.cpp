// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLogUtils.h"

#include "CoreGlobals.h"
#include "GenericPlatform/GenericPlatformStackWalk.h"
#include "HAL/UnrealMemory.h"
#include "Misc/Char.h"
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

bool SentryLogUtils::IsCallstackLine(const FString& Line)
{
	// Frame lines produced by FDebug::LogFormattedMessageWithCallstack
	if (Line.Contains(TEXT("[Callstack]")))
	{
		return true;
	}

	// Frame lines in Apple's backtrace format (e.g. "5   MyGame   0x00000001052042b8 FuncName + 248") which
	// don't get the prefix above because they don't start with "0x" (i.e. iOS uses NSThread callStackSymbols)
	const TCHAR* Ptr = *Line;

	if (!FChar::IsDigit(*Ptr))
	{
		return false;
	}
	while (FChar::IsDigit(*Ptr))
	{
		++Ptr;
	}

	if (!FChar::IsWhitespace(*Ptr))
	{
		return false;
	}
	while (FChar::IsWhitespace(*Ptr))
	{
		++Ptr;
	}

	if (*Ptr == TEXT('\0'))
	{
		return false;
	}
	while (*Ptr != TEXT('\0') && !FChar::IsWhitespace(*Ptr))
	{
		++Ptr;
	}

	if (!FChar::IsWhitespace(*Ptr))
	{
		return false;
	}
	while (FChar::IsWhitespace(*Ptr))
	{
		++Ptr;
	}

	return Ptr[0] == TEXT('0') && (Ptr[1] == TEXT('x') || Ptr[1] == TEXT('X')) && FChar::IsHexDigit(Ptr[2]);
}
