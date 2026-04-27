// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryModule.h"
#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

// SENTRY_WINGDK is explicitly defined only for Windows/WinGDK, so default it to 0 for other platforms to avoid -Wundef
#ifndef SENTRY_WINGDK
#define SENTRY_WINGDK 0
#endif

#define LOCTEXT_NAMESPACE "FSentryModule"

const FName FSentryModule::ModuleName = "Sentry";

void FSentryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	SentrySettings = NewObject<USentrySettings>(GetTransientPackage(), "SentrySettings", RF_Standalone);
	SentrySettings->AddToRoot();

#if PLATFORM_MAC && !USE_SENTRY_NATIVE
	// Load Sentry Cocoa dynamic library
	FString LibraryPath = FPaths::Combine(GetBinariesPath(), TEXT("sentry.dylib"));
	mDllHandleSentry = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (!mDllHandleSentry)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to load sentry.dylib from %s"), *LibraryPath);
	}
#endif

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Sentry",
			LOCTEXT("RuntimeSettingsName", "Sentry"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Sentry"),
			SentrySettings);
	}
}

void FSentryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

#if PLATFORM_MAC && !USE_SENTRY_NATIVE
	// Free sentry dynamic library
	if (mDllHandleSentry)
	{
		FPlatformProcess::FreeDllHandle(mDllHandleSentry);
		mDllHandleSentry = nullptr;
	}
#endif

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Sentry");
	}

	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		SentrySettings->RemoveFromRoot();
	}
	else
	{
		SentrySettings = nullptr;
	}
}

FSentryModule& FSentryModule::Get()
{
	return FModuleManager::LoadModuleChecked<FSentryModule>(ModuleName);
}

bool FSentryModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded(ModuleName);
}

USentrySettings* FSentryModule::GetSettings() const
{
	return SentrySettings;
}

FString FSentryModule::GetPluginPath()
{
	return IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
}

FString FSentryModule::GetBinariesPath()
{
	// Unreal Engine currently stores Windows ARM64 binaries in the Win64 directory and does not distinguish by architecture,
	// so when building the path to platform-specific plugin resources, the platform name must be set manually
	// instead of relying on FPlatformProcess::GetBinariesSubdirectory().
	// The same applies to WinGDK targets, which the engine treats as Win64 by default in this context.

#if SENTRY_WINGDK
	const FString PlatformDir = TEXT("WinGDK");
#elif PLATFORM_WINDOWS && PLATFORM_CPU_ARM_FAMILY
	const FString PlatformDir = TEXT("WinArm64");
#else
	const FString PlatformDir = FPlatformProcess::GetBinariesSubdirectory();
#endif

	return FPaths::Combine(GetPluginPath(), TEXT("Binaries"), PlatformDir);
}

FString FSentryModule::GetThirdPartyPath()
{
	// Unreal Engine currently stores Windows ARM64 binaries in the Win64 directory and does not distinguish by architecture,
	// so when building the path to platform-specific plugin resources, the platform name must be set manually
	// instead of relying on FPlatformProcess::GetBinariesSubdirectory().
	// The same applies to WinGDK targets, which the engine treats as Win64 by default in this context.

#if SENTRY_WINGDK
	const FString PlatformDir = TEXT("WinGDK");
#elif PLATFORM_WINDOWS && PLATFORM_CPU_ARM_FAMILY
	const FString PlatformDir = TEXT("WinArm64");
#else
	const FString PlatformDir = FPlatformProcess::GetBinariesSubdirectory();
#endif

	return FPaths::Combine(GetPluginPath(), TEXT("Source"), TEXT("ThirdParty"), PlatformDir);
}

FString FSentryModule::GetPluginVersion()
{
	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin(TEXT("Sentry"));

	if (!plugin)
	{
		return FString();
	}

	return plugin->GetDescriptor().VersionName;
}

bool FSentryModule::IsMarketplaceVersion()
{
	// Marketplace version check heuristic:
	// In case the plugin installed via Epic Games launcher path to its base dir supposed to be <EngineDir>/Plugins/Marketplace/Sentry
	// This approach is not foolproof as users may manually copy plugin to this location

	const FString PluginPath = FPaths::ConvertRelativePathToFull(
		IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir());
	const FString MarketplacePrefix = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("Marketplace")));

	return PluginPath.StartsWith(MarketplacePrefix);
}

#if PLATFORM_MAC && !USE_SENTRY_NATIVE

void* FSentryModule::GetSentryLibHandle() const
{
	return mDllHandleSentry;
}

Class FSentryModule::GetSentryCocoaClass(const ANSICHAR* ClassName)
{
	ANSICHAR ClassNamePattern[256];
	FCStringAnsi::Snprintf(ClassNamePattern, sizeof(ClassNamePattern), "OBJC_CLASS_$_%s", ClassName);
	Class FoundClass = (__bridge Class)dlsym(GetSentryLibHandle(), ClassNamePattern);

	if (!FoundClass)
	{
		// Try to load with Swift mangled class name
		ANSICHAR MangledClassNamePattern[256];
		int32 ClassNameLen = FCStringAnsi::Strlen(ClassName);
		FCStringAnsi::Snprintf(MangledClassNamePattern, sizeof(MangledClassNamePattern), "OBJC_CLASS_$__TtC6Sentry%d%s", ClassNameLen, ClassName);
		FoundClass = (__bridge Class)dlsym(GetSentryLibHandle(), MangledClassNamePattern);
	}

	return FoundClass;
}

#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentryModule, Sentry)

DEFINE_LOG_CATEGORY(LogSentrySdk);
