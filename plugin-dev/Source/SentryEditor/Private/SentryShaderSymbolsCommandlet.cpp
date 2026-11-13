// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryShaderSymbolsCommandlet.h"

#include "SentryShaderSymbols.h"

#include "Misc/Parse.h"
#include "Misc/Paths.h"

USentryShaderSymbolsCommandlet::USentryShaderSymbolsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 USentryShaderSymbolsCommandlet::Main(const FString& Params)
{
	return FSentryShaderSymbols::Run(FPaths::ProjectSavedDir(), FParse::Param(*Params, TEXT("Upload")));
}
