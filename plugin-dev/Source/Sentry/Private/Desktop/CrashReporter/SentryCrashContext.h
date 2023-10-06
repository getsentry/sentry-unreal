﻿// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Runtime/Launch/Resources/Version.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

#if USE_SENTRY_NATIVE

class SentryScopeDesktop;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
class FSentryCrashContext
#else
class FSentryCrashContext : public FGenericCrashContext
#endif
{
	FSentryCrashContext();

public:
	static FSentryCrashContext Get();

	void Apply(TSharedPtr<SentryScopeDesktop> Scope);

	FString GetGameData(const FString& Key);

private:
	FSharedCrashContext CrashContext;
};

#endif