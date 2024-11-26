// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Misc/EngineVersionComparison.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

#if USE_SENTRY_NATIVE

class SentryScopeDesktop;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
struct FSentryCrashContext : public FGenericCrashContext
#else
struct FSentryCrashContext
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