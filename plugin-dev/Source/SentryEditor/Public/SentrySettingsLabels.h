// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IDetailLayoutBuilder;
class IDetailPropertyRow;
class IPropertyHandle;
class SHorizontalBox;
class SWidget;

/**
 * Helper that adds labels to Sentry settings rows based on the property metadata:
 * - SentryPlatforms: comma-separated platforms where the setting has an effect, or "!Platform" tokens for platforms where it has none
 * - SentryMinEngine: minimum engine version (e.g. "5.5")
 * - SentryRequiresRebuild: the build reads the setting
 * - SentryExperimental: the feature is experimental
 */
class FSentrySettingsLabels
{
public:
	static void AddLabels(IDetailLayoutBuilder& DetailBuilder);

private:
	static void AddLabels(IDetailPropertyRow& Row, TSharedRef<IPropertyHandle> Handle, const FProperty* Property);

	static void AddPlatformLabels(const FProperty* Property, SHorizontalBox& Labels, FString& FilterString);
	static void AddEngineLabel(const FProperty* Property, IDetailPropertyRow& Row, SHorizontalBox& Labels, FString& FilterString);
	static void AddRebuildLabel(SHorizontalBox& Labels, FString& FilterString);
	static void AddExperimentalLabel(SHorizontalBox& Labels, FString& FilterString);

	static void AddLabel(SHorizontalBox& Labels, const FText& Text, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor);

	static bool ShouldAddLabels(const FProperty* Property);

	static bool GetPlatformNames(const FString& Token, FText& OutShortName, FText& OutName);
	static bool IsEngineVersionSupported(const FString& MinVersion);

	static TSharedRef<SWidget> MakeLabel(const FText& Text, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor);

	static const FName PlatformsMeta;
	static const FName MinEngineMeta;
	static const FName RequiresRebuildMeta;
	static const FName ExperimentalMeta;

	static const FLinearColor PlatformLabelColor;
	static const FLinearColor PlatformLabelTextColor;
	static const FLinearColor ExcludedPlatformLabelTextColor;
	static const FLinearColor EngineLabelColor;
	static const FLinearColor EngineLabelTextColor;
	static const FLinearColor UnsupportedEngineLabelColor;
	static const FLinearColor UnsupportedEngineLabelTextColor;
	static const FLinearColor RebuildLabelColor;
	static const FLinearColor RebuildLabelTextColor;
	static const FLinearColor ExperimentalLabelColor;
	static const FLinearColor ExperimentalLabelTextColor;
};
