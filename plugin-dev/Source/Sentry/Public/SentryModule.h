// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class USentrySettings;

class SENTRY_API FSentryModule : public IModuleInterface
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

	/** Gets internal settings object to support runtime configuration changes. */
	USentrySettings* GetSettings() const;

	/** Gets path to plugin's binaries folder for current platform. */
	FString GetBinariesPath();

	/** Gets path to plugin's third-party resources folder for current platform. */
	FString GetThirdPartyPath();

	/** Gets plugin's version. */
	static FString GetPluginVersion();

	/** Gets flag indicating whether plugin was downloaded from UE Marketplace. */
	static bool IsMarketplaceVersion();

#if PLATFORM_MAC
	/** Gets handle to dynamically loaded sentry library. */
	void* GetSentryLibHandle() const;
#endif

	static const FName ModuleName;

	static const bool IsMarketplace;

private:
	USentrySettings* SentrySettings = nullptr;

#if PLATFORM_MAC
	void* mDllHandleSentry = nullptr;
#endif
};
