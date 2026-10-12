// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IDetailLayoutBuilder;
class IDetailPropertyRow;
class IPropertyHandle;
class SHorizontalBox;
class SWidget;

/**
 * Helper that adds badges to Sentry settings rows based on the property metadata:
 * - SentryPlatforms: comma-separated platforms where the setting has an effect, or "!Platform" tokens for platforms where it has none
 * - SentryMinEngine: minimum engine version (e.g. "5.5")
 * - SentryRequiresRebuild: the build reads the setting
 * - SentryExperimental: the feature is experimental
 */
class FSentrySettingsBadges
{
public:
	static void AddBadges(IDetailLayoutBuilder& DetailBuilder);

private:
	static void AddBadges(IDetailPropertyRow& Row, TSharedRef<IPropertyHandle> Handle, const FProperty* Property);

	static void AddPlatformBadges(const FProperty* Property, SHorizontalBox& Badges, FString& FilterString);
	static void AddEngineBadge(const FProperty* Property, IDetailPropertyRow& Row, SHorizontalBox& Badges, FString& FilterString);
	static void AddRebuildBadge(SHorizontalBox& Badges, FString& FilterString);
	static void AddExperimentalBadge(SHorizontalBox& Badges, FString& FilterString);

	static void AddBadge(SHorizontalBox& Badges, const FText& Label, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor);

	static bool ShouldAddBadges(const FProperty* Property);

	static bool GetPlatformLabel(const FString& Token, FText& OutLabel, FText& OutName);
	static bool IsEngineVersionSupported(const FString& MinVersion);

	static TSharedRef<SWidget> MakeBadge(const FText& Label, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor);

	static const FName PlatformsMeta;
	static const FName MinEngineMeta;
	static const FName RequiresRebuildMeta;
	static const FName ExperimentalMeta;

	static const FLinearColor PlatformBadgeColor;
	static const FLinearColor PlatformBadgeTextColor;
	static const FLinearColor ExcludedPlatformBadgeTextColor;
	static const FLinearColor EngineBadgeColor;
	static const FLinearColor EngineBadgeTextColor;
	static const FLinearColor UnsupportedEngineBadgeColor;
	static const FLinearColor UnsupportedEngineBadgeTextColor;
	static const FLinearColor RebuildBadgeColor;
	static const FLinearColor RebuildBadgeTextColor;
	static const FLinearColor ExperimentalBadgeColor;
	static const FLinearColor ExperimentalBadgeTextColor;
};
