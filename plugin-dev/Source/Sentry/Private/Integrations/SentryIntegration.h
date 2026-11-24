// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USentrySettings;

/**
 * SDK-owned engine instrumentation (automatic breadcrumbs, log capturing, etc.).
 *
 * The underlying platform SDK is guaranteed to be enabled between Setup() and Teardown().
 */
class ISentryIntegration
{
public:
	virtual ~ISentryIntegration() = default;

	/** Name of the integration used for logging. */
	virtual const TCHAR* GetName() const = 0;

	/**
	 * Installs the integration. Settings are valid only during this call - copy any values needed later.
	 *
	 * @return False if the integration is not needed (e.g. disabled in settings).
	 */
	virtual bool Setup(const USentrySettings& Settings) = 0;

	/**
	 * Uninstalls the integration. Called even if Setup() returned false and must be idempotent.
	 * No integration callbacks may run after this returns (unbind delegates, stop threads, etc.).
	 */
	virtual void Teardown() = 0;
};
