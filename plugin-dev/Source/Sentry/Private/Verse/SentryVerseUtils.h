// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Verse/SentryScriptStackFrame.h"

class FSentryVerseUtils
{
public:
	/** Removes the wrapper that Verse adds around the message passed to Err(). */
	static FString GetUserMessage(const FString& Message);

	/** Parses the callstack from a formatted Verse runtime error text. Frames are ordered from oldest to most recent call. */
	static TArray<FSentryScriptStackFrame> ParseCallstack(const FString& RuntimeErrorText);

	/** Fills source context for frames whose Verse source file can be found on disk. */
	static void AddSourceContext(TArray<FSentryScriptStackFrame>& Frames);

private:
	static constexpr const TCHAR* SourceMarker = TEXT("\t(Source: ");
	static constexpr const TCHAR* UserMessagePrefix = TEXT("User Message: '");
	static constexpr int32 SourceContextLines = 5;

	static bool ParseFrameLine(const FString& Line, FSentryScriptStackFrame& OutFrame);
	static void ParseFunctionName(const FString& FullName, FSentryScriptStackFrame& OutFrame);
	static FString ResolveSourcePath(const FString& VerseFilePath);
	static const TArray<FString>* GetSourceLines(const FString& VerseFilePath);
};
