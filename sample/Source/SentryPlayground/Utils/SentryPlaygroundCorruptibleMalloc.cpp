// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundCorruptibleMalloc.h"

#include "Runtime/Launch/Resources/Version.h"

void FSentryCorruptibleMalloc::ArmForNextCrash()
{
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
	FSentryCorruptibleMalloc* Proxy = new FSentryCorruptibleMalloc(UE::Private::GMalloc);
	Proxy->bArmed = true;
	UE::Private::GMalloc = Proxy;
#else
	FSentryCorruptibleMalloc* Proxy = new FSentryCorruptibleMalloc(GMalloc);
	Proxy->bArmed = true;
	GMalloc = Proxy;
#endif
}
