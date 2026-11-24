// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

class ISentrySubsystem;
class UWorld;

/**
 * Adds breadcrumbs automatically for game events enabled in plugin settings
 * (map loading, game state class, user activity string and game session ID changes).
 */
class FSentryBreadcrumbsIntegration final : public ISentryIntegration
{
public:
	explicit FSentryBreadcrumbsIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);

	virtual const TCHAR* GetName() const override { return TEXT("AutomaticBreadcrumbs"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMapWithWorld(UWorld* World);
	void OnGameStateClassChanged(const FString& GameState);
	void OnUserActivityStringChanged(const FString& Activity);
	void OnGameSessionIDChanged(const FString& SessionId);

	void AddBreadcrumb(const FString& Message, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level = ESentryLevel::Info);

	TSharedRef<ISentrySubsystem> NativeImpl;
};
