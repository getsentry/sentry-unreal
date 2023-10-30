// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Runtime/Launch/Resources/Version.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

#if USE_SENTRY_NATIVE

class SentryScopeDesktop;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
struct FSentryCrashContext
#else
struct FSentryCrashContext : public FGenericCrashContext
#endif
{
	FSentryCrashContext(TSharedPtr<FSharedCrashContext> Context);

public:
	static TSharedPtr<FSentryCrashContext> Get();

	void Apply(TSharedPtr<SentryScopeDesktop> Scope);

	FString GetGameData(const FString& Key);

private:
	TSharedPtr<FSharedCrashContext> CrashContext;
};

#endif