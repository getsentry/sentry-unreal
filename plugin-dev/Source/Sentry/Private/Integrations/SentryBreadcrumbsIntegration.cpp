// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryBreadcrumbsIntegration.h"

#include "SentrySettings.h"

#include "Interface/SentrySubsystemInterface.h"

#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "UObject/UObjectGlobals.h"

FSentryBreadcrumbsIntegration::FSentryBreadcrumbsIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

bool FSentryBreadcrumbsIntegration::Setup(const USentrySettings& Settings)
{
	const FAutomaticBreadcrumbs& Flags = Settings.AutomaticBreadcrumbs;

	if (!Flags.bOnMapLoadingStarted && !Flags.bOnMapLoaded && !Flags.bOnGameStateClassChanged &&
		!Flags.bOnUserActivityStringChanged && !Flags.bOnGameSessionIDChanged)
	{
		return false;
	}

	if (Flags.bOnMapLoadingStarted)
	{
		FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FSentryBreadcrumbsIntegration::OnPreLoadMap);
	}

	if (Flags.bOnMapLoaded)
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSentryBreadcrumbsIntegration::OnPostLoadMapWithWorld);
	}

	if (Flags.bOnGameStateClassChanged)
	{
		FCoreDelegates::GameStateClassChanged.AddRaw(this, &FSentryBreadcrumbsIntegration::OnGameStateClassChanged);
	}

	if (Flags.bOnUserActivityStringChanged)
	{
		FCoreDelegates::UserActivityStringChanged.AddRaw(this, &FSentryBreadcrumbsIntegration::OnUserActivityStringChanged);
	}

	if (Flags.bOnGameSessionIDChanged)
	{
		FCoreDelegates::GameSessionIDChanged.AddRaw(this, &FSentryBreadcrumbsIntegration::OnGameSessionIDChanged);
	}

	return true;
}

void FSentryBreadcrumbsIntegration::Teardown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FCoreDelegates::GameStateClassChanged.RemoveAll(this);
	FCoreDelegates::UserActivityStringChanged.RemoveAll(this);
	FCoreDelegates::GameSessionIDChanged.RemoveAll(this);
}

void FSentryBreadcrumbsIntegration::OnPreLoadMap(const FString& MapName)
{
	AddBreadcrumb(TEXT("PreLoadMap"), { { TEXT("Map"), MapName } });
}

void FSentryBreadcrumbsIntegration::OnPostLoadMapWithWorld(UWorld* World)
{
	if (World)
	{
		AddBreadcrumb(TEXT("PostLoadMapWithWorld"), { { TEXT("Map"), World->GetMapName() } });
	}
	else
	{
		AddBreadcrumb(TEXT("PostLoadMapWithWorld"), { { TEXT("Error"), TEXT("Map load failed") } }, ESentryLevel::Error);
	}
}

void FSentryBreadcrumbsIntegration::OnGameStateClassChanged(const FString& GameState)
{
	AddBreadcrumb(TEXT("GameStateClassChanged"), { { TEXT("GameState"), GameState } });
}

void FSentryBreadcrumbsIntegration::OnUserActivityStringChanged(const FString& Activity)
{
	AddBreadcrumb(TEXT("UserActivityStringChanged"), { { TEXT("Activity"), Activity } });
}

void FSentryBreadcrumbsIntegration::OnGameSessionIDChanged(const FString& SessionId)
{
	AddBreadcrumb(TEXT("GameSessionIDChanged"), { { TEXT("Session ID"), SessionId } });
}

void FSentryBreadcrumbsIntegration::AddBreadcrumb(const FString& Message, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	NativeImpl->AddBreadcrumbWithParams(Message, TEXT("Unreal"), TEXT("Default"), Data, Level);
}
