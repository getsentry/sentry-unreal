// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySDK.h"
#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Modules/ModuleManager.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FSentrySDKModule"

const FName FSentrySDKModule::ModuleName = "SentrySDK";

void FSentrySDKModule::StartupModule()
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
}

void FSentrySDKModule::ShutdownModule()
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
}

FSentrySDKModule& FSentrySDKModule::Get()
{
	return FModuleManager::LoadModuleChecked<FSentrySDKModule>(ModuleName);
}

bool FSentrySDKModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded(ModuleName);
}

USentrySettings* FSentrySDKModule::GetSettings() const
{
	return SentrySettings;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentrySDKModule, SentrySDK)

DEFINE_LOG_CATEGORY(LogSentrySdk);