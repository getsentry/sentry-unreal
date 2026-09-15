// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryEditorLibrary.h"

#include "SentryModule.h"
#include "SentrySettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogSentryEditor, Log, All);

USentrySettings* USentryEditorLibrary::GetActiveSettings()
{
	return FSentryModule::IsAvailable() ? FSentryModule::Get().GetSettings() : nullptr;
}

bool USentryEditorLibrary::SaveSettingToConfig(USentrySettings* Settings, FName PropertyName)
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

ESentryDsnSource USentryEditorLibrary::GetDsnSource()
{
	const USentrySettings* Settings = GetActiveSettings();
	if (!Settings)
	{
		return ESentryDsnSource::None;
	}

	if (GIsEditor && !Settings->EditorDsn.IsEmpty())
	{
		return ESentryDsnSource::EditorDsn;
	}

	if (!Settings->Dsn.IsEmpty())
	{
		return ESentryDsnSource::Dsn;
	}

	if (!FPlatformMisc::GetEnvironmentVariable(TEXT("SENTRY_DSN")).IsEmpty())
	{
		return ESentryDsnSource::EnvironmentVariable;
	}

	return ESentryDsnSource::None;
}
