// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "Misc/EngineVersionComparison.h"

#if USE_SENTRY_NATIVE

class ISentryScope;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
struct FGenericPlatformSentryCrashContext : public FGenericCrashContext
#else
struct FGenericPlatformSentryCrashContext
#endif
{
	FGenericPlatformSentryCrashContext(TSharedPtr<FSharedCrashContext> Context);

public:
	static TSharedPtr<FGenericPlatformSentryCrashContext> Get();

	void Apply(TSharedPtr<ISentryScope> Scope);

	FString GetGameData(const FString& Key);

private:
	TSharedPtr<FSharedCrashContext> CrashContext;
};

#endif