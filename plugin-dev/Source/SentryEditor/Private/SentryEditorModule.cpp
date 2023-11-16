// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEditorModule.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryEngineSubsystem.h"
#include "SentrySettingsCustomization.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Editor.h"

#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "FSentryEditorModule"

const FName FSentryEditorModule::ModuleName = "SentryEditor";

void FSentryEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("SentrySettings", FOnGetDetailCustomizationInstance::CreateStatic(&FSentrySettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->CrashCapturingMode == ESentryCrashCapturingMode::GameOnly)
	{
		PieSessionStartedDelegate = FEditorDelegates::BeginPIE.AddRaw(this, &FSentryEditorModule::OnBeginPIE);
		PieSessionEndedDelegate = FEditorDelegates::EndPIE.AddRaw(this, &FSentryEditorModule::OnEndPIE);
	}
}

void FSentryEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if(PieSessionEndedDelegate.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(PieSessionStartedDelegate);
		PieSessionStartedDelegate.Reset();
	}

	if(PieSessionEndedDelegate.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(PieSessionEndedDelegate);
		PieSessionEndedDelegate.Reset();
	}
}

FSentryEditorModule& FSentryEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FSentryEditorModule>(ModuleName);
}

void FSentryEditorModule::OnBeginPIE(bool bIsSimulating)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->InitAutomatically)
	{
		GEngine->GetEngineSubsystem<USentryEngineSubsystem>()->Initialize();
	}
}

void FSentryEditorModule::OnEndPIE(bool bIsSimulating)
{
	GEngine->GetEngineSubsystem<USentryEngineSubsystem>()->Close();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSentryEditorModule, SentryEditor)
