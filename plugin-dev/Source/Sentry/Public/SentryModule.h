// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class USentrySettings;

class FSentryModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might be already unloaded.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed.
	 */
	static FSentryModule& Get();

	/**
	 * Checks if this module is loaded and ready. It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use.
	 */
	static bool IsAvailable();

	/** Gets Sentry library handle for manual symbols loading. */
	void* GetSentryLibHandle() const;

	/** Gets internal settings object to support runtime configuration changes. */
	USentrySettings* GetSettings() const;

	/** Gets plugin's version. */
	static FString GetPluginVersion();

	static const FName ModuleName;

private:
	USentrySettings* SentrySettings = nullptr;

	void* mDllHandleSentry;
};
