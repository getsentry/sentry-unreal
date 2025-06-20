// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryEditorModule.h"
#include "SentrySettings.h"
#include "SentrySettingsCustomization.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FSentryEditorModule"

const FName FSentryEditorModule::ModuleName = "SentryEditor";

void FSentryEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("SentrySettings", FOnGetDetailCustomizationInstance::CreateStatic(&FSentrySettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FSentryEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

FSentryEditorModule& FSentryEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FSentryEditorModule>(ModuleName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentryEditorModule, SentryEditor)
