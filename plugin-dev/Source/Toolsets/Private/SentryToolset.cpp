// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryToolset.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
USentrySubsystem* GetSentrySubsystem()
{
	if (!GEngine)
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Engine is not available."));
		return nullptr;
	}

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!SentrySubsystem)
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Sentry subsystem is not available."));
	}

	return SentrySubsystem;
}

USentrySettings* GetSentrySettings()
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();
	if (!Settings)
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Sentry settings are not available."));
	}

	return Settings;
}
} // namespace

bool USentryToolset::IsSentryEnabled()
{
	USentrySubsystem* SentrySubsystem = GetSentrySubsystem();
	if (!SentrySubsystem)
	{
		return false;
	}

	return SentrySubsystem->IsEnabled();
}

bool USentryToolset::IsDsnConfigured()
{
	const USentrySettings* Settings = GetSentrySettings();
	if (!Settings)
	{
		return false;
	}

	return !Settings->Dsn.IsEmpty();
}

void USentryToolset::SetDsn(const FString& Dsn)
{
	if (Dsn.IsEmpty())
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Dsn must not be empty."));
		return;
	}

	USentrySettings* Settings = GetSentrySettings();
	if (!Settings)
	{
		return;
	}

	Settings->Dsn = Dsn;

	// Persist just this entry, leaving the rest of the section alone.
	const FProperty* DsnProperty = USentrySettings::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(USentrySettings, Dsn));
	Settings->UpdateSinglePropertyInConfigFile(DsnProperty, Settings->GetDefaultConfigFilename());
}

bool USentryToolset::ReinitializeSentry()
{
	USentrySubsystem* SentrySubsystem = GetSentrySubsystem();
	if (!SentrySubsystem)
	{
		return false;
	}

	// Initialize() shuts down a running session before starting a new one.
	SentrySubsystem->Initialize();

	return SentrySubsystem->IsEnabled();
}

FString USentryToolset::CaptureTestMessage(const FString& Message)
{
	if (Message.IsEmpty())
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Message must not be empty."));
		return FString();
	}

	USentrySubsystem* SentrySubsystem = GetSentrySubsystem();
	if (!SentrySubsystem)
	{
		return FString();
	}

	if (!SentrySubsystem->IsEnabled())
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Sentry SDK is not running, so no event can be captured."));
		return FString();
	}

	return SentrySubsystem->CaptureMessage(Message);
}
