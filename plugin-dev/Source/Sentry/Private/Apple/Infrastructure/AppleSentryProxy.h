// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Containers/UnrealString.h"

#include "Convenience/AppleSentryInclude.h"

class FAppleSentryProxy
{
public:
	/** Creates a URL session that routes requests through the given proxy, or nil if the proxy URL is invalid or unsupported */
	static NSURLSession* CreateUrlSession(const FString& proxyUrl);
};

#endif // !USE_SENTRY_NATIVE
