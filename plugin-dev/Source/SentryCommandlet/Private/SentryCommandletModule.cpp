// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryCommandletModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FSentryCommandletModule, SentryCommandlet)

void FSentryCommandletModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSentryCommandletModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}