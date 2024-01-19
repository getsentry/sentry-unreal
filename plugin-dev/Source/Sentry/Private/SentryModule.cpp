// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryModule.h"
#include "SentryDefines.h"
#include "SentrySettings.h"

#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FSentryModule"

const FName FSentryModule::ModuleName = "Sentry";

const bool FSentryModule::IsMarketplace = false;

void FSentryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	SentrySettings = NewObject<USentrySettings>(GetTransientPackage(), "SentrySettings", RF_Standalone);
	SentrySettings->AddToRoot();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Sentry",
			LOCTEXT("RuntimeSettingsName", "Sentry"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Sentry"),
			SentrySettings);
	}

#if PLATFORM_MAC
	const FString SentryLibName = TEXT("sentry.dylib");
	const FString BinariesDirPath = GIsEditor ? FPaths::Combine(GetThirdPartyPath(), TEXT("bin")) : GetBinariesPath();

	FPlatformProcess::PushDllDirectory(*BinariesDirPath);
	mDllHandleSentry = FPlatformProcess::GetDllHandle(*FPaths::Combine(BinariesDirPath, SentryLibName));
	FPlatformProcess::PopDllDirectory(*BinariesDirPath);
#endif
}

void FSentryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

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

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (mDllHandleSentry)
	{
		FPlatformProcess::FreeDllHandle(mDllHandleSentry);
		mDllHandleSentry = nullptr;
	}
#endif
}

FSentryModule& FSentryModule::Get()
{
	return FModuleManager::LoadModuleChecked<FSentryModule>(ModuleName);
}

bool FSentryModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded(ModuleName);
}

void* FSentryModule::GetSentryLibHandle() const
{
	return mDllHandleSentry;
}

USentrySettings* FSentryModule::GetSettings() const
{
	return SentrySettings;
}

FString FSentryModule::GetBinariesPath()
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();

	return FPaths::Combine(PluginDir, TEXT("Binaries"), FPlatformProcess::GetBinariesSubdirectory());
}

FString FSentryModule::GetThirdPartyPath()
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();

	return FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), FPlatformProcess::GetBinariesSubdirectory());
}

FString FSentryModule::GetPluginVersion()
{
	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin(TEXT("Sentry"));

	if(!plugin)
	{
		return FString();
	}

	return plugin->GetDescriptor().VersionName;
}

bool FSentryModule::IsMarketplaceVersion()
{
	return IsMarketplace;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentryModule, Sentry)

DEFINE_LOG_CATEGORY(LogSentrySdk);