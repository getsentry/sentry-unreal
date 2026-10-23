// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Stack frame of a scripting language (e.g. Verse) that carries symbolic location info instead of an instruction address. */
struct FSentryScriptStackFrame
{
	FString Function;
	FString RawFunction;
	FString Module;
	FString Filename;
	int32 LineNumber = 0;
	int32 ColumnNumber = 0;
	FString ContextLine;
	TArray<FString> PreContext;
	TArray<FString> PostContext;
};
