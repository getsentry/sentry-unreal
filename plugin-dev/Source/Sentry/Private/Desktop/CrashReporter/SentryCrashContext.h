#pragma once

#include "GenericPlatform/GenericPlatformCrashContext.h"

class SentryScopeDesktop;

class FSentryCrashContext : public FGenericCrashContext
{
public:
	FSentryCrashContext(const FSharedCrashContext& Context);

	void Apply(TSharedPtr<SentryScopeDesktop> Scope);

private:
	FSharedCrashContext CrashContext;
};
