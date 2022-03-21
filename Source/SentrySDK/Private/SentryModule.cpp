// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryModule.h"
#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Developer/Settings/Public/ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FSentryModule"

const FName FSentryModule::ModuleName = "SentrySDK";

void FSentryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	SentrySettings = NewObject<USentrySettings>(GetTransientPackage(), "SentrySettings", RF_Standalone);
	SentrySettings->AddToRoot();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "SentrySdk",
			LOCTEXT("RuntimeSettingsName", "Sentry SDK"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Sentry SDK"),
			SentrySettings);
	}

#if PLATFORM_WINDOWS
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("SentrySDK"))->GetBaseDir();
	const FString DLLPath = PluginDir / TEXT("Binaries/Win64/");

	FPlatformProcess::PushDllDirectory(*DLLPath);

	mDllHandleSentry = FPlatformProcess::GetDllHandle(*(DLLPath + "sentry.dll"));

	FPlatformProcess::PopDllDirectory(*DLLPath);
#endif
#if PLATFORM_MAC
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("SentrySDK"))->GetBaseDir();
	const FString DLLPath = PluginDir / TEXT("Binaries/Mac/");

	FPlatformProcess::PushDllDirectory(*DLLPath);

	mDllHandleSentry = FPlatformProcess::GetDllHandle(*(DLLPath + "sentry.dylib"));

	FPlatformProcess::PopDllDirectory(*DLLPath);
#endif
}

void FSentryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "SentrySdk");
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

#if PLATFORM_WINDOWS || PLATFORM_MAC
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

USentrySettings* FSentryModule::GetSettings() const
{
	return SentrySettings;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentryModule, SentrySDK)

DEFINE_LOG_CATEGORY(LogSentrySdk);