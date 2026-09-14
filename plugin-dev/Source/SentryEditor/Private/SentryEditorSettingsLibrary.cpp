// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryEditorSettingsLibrary.h"

#include "SentryModule.h"
#include "SentrySettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogSentryEditor, Log, All);

USentrySettings* USentryEditorSettingsLibrary::GetActiveSettings()
{
	return FSentryModule::IsAvailable() ? FSentryModule::Get().GetSettings() : nullptr;
}

bool USentryEditorSettingsLibrary::SaveSettingToConfig(USentrySettings* Settings, FName PropertyName)
{
	if (!Settings)
	{
		UE_LOG(LogSentryEditor, Warning, TEXT("Invalid Sentry settings object - nothing was saved."));
		return false;
	}

	const FProperty* Property = Settings->GetClass()->FindPropertyByName(PropertyName);
	if (!Property)
	{
		UE_LOG(LogSentryEditor, Warning, TEXT("Unknown Sentry setting `%s` - nothing was saved."), *PropertyName.ToString());
		return false;
	}

	Settings->UpdateSinglePropertyInConfigFile(Property, Settings->GetDefaultConfigFilename());

	return true;
}
