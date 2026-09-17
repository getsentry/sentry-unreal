// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryEditorLibrary.h"

#include "SentryModule.h"
#include "SentrySettings.h"

#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogSentryEditor, Log, All);

namespace
{

// Both spellings of a setting name compare equal once case and underscores are ignored.
FString NormalizeSettingName(const FString& Name)
{
	return Name.Replace(TEXT("_"), TEXT("")).ToLower();
}

// Mirrors how Unreal's Python API names a property: boolean properties lose their `b` prefix
// (`bRequireUserConsent` is exposed as `require_user_consent`).
FString NormalizePropertyName(const FProperty* Property)
{
	FString Name = Property->GetName();

	if (Property->IsA<FBoolProperty>() && Name.Len() > 1 && Name[0] == TEXT('b') && FChar::IsUpper(Name[1]))
	{
		Name.RightChopInline(1);
	}

	return NormalizeSettingName(Name);
}

} // namespace

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

	const FName ResolvedName = ResolveSettingName(PropertyName);
	const FProperty* Property = ResolvedName.IsNone() ? nullptr : Settings->GetClass()->FindPropertyByName(ResolvedName);
	if (!Property)
	{
		UE_LOG(LogSentryEditor, Warning, TEXT("Unknown Sentry setting `%s` - nothing was saved."), *PropertyName.ToString());
		return false;
	}

	Settings->UpdateSinglePropertyInConfigFile(Property, Settings->GetDefaultConfigFilename());

	return true;
}

FName USentryEditorLibrary::ResolveSettingName(FName PropertyName)
{
	const UClass* SettingsClass = USentrySettings::StaticClass();

	if (SettingsClass->FindPropertyByName(PropertyName))
	{
		return PropertyName;
	}

	const FString NormalizedName = NormalizeSettingName(PropertyName.ToString());

	for (TFieldIterator<FProperty> It(SettingsClass); It; ++It)
	{
		if (NormalizePropertyName(*It) == NormalizedName)
		{
			return It->GetFName();
		}
	}

	return NAME_None;
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
