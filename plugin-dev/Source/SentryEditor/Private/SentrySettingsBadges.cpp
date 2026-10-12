// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentrySettingsBadges.h"
#include "SentrySettings.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"

#include "Runtime/Launch/Resources/Version.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogSentrySettingsBadges, Log, All);

const FName FSentrySettingsBadges::PlatformsMeta(TEXT("SentryPlatforms"));
const FName FSentrySettingsBadges::MinEngineMeta(TEXT("SentryMinEngine"));
const FName FSentrySettingsBadges::RequiresRebuildMeta(TEXT("SentryRequiresRebuild"));
const FName FSentrySettingsBadges::ExperimentalMeta(TEXT("SentryExperimental"));

const FLinearColor FSentrySettingsBadges::PlatformBadgeColor(FColor(40, 46, 54));
const FLinearColor FSentrySettingsBadges::PlatformBadgeTextColor(FColor(150, 160, 172));
const FLinearColor FSentrySettingsBadges::ExcludedPlatformBadgeTextColor(FColor(214, 150, 140));
const FLinearColor FSentrySettingsBadges::EngineBadgeColor(FColor(46, 42, 58));
const FLinearColor FSentrySettingsBadges::EngineBadgeTextColor(FColor(165, 155, 188));
const FLinearColor FSentrySettingsBadges::UnsupportedEngineBadgeColor(FColor(128, 34, 34));
const FLinearColor FSentrySettingsBadges::UnsupportedEngineBadgeTextColor(FColor(255, 205, 205));
const FLinearColor FSentrySettingsBadges::RebuildBadgeColor(FColor(122, 80, 18));
const FLinearColor FSentrySettingsBadges::RebuildBadgeTextColor(FColor(255, 218, 160));
const FLinearColor FSentrySettingsBadges::ExperimentalBadgeColor(FColor(24, 66, 72));
const FLinearColor FSentrySettingsBadges::ExperimentalBadgeTextColor(FColor(150, 220, 226));

void FSentrySettingsBadges::AddBadges(IDetailLayoutBuilder& DetailBuilder)
{
	for (TFieldIterator<FProperty> It(USentrySettings::StaticClass()); It; ++It)
	{
		const FProperty* Property = *It;
		if (!ShouldAddBadges(Property))
		{
			continue;
		}

		TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Property->GetFName());
		if (IDetailPropertyRow* Row = DetailBuilder.EditDefaultProperty(Handle))
		{
			AddBadges(*Row, Handle, Property);
		}
	}
}

void FSentrySettingsBadges::AddBadges(IDetailPropertyRow& Row, TSharedRef<IPropertyHandle> Handle, const FProperty* Property)
{
	TSharedRef<SHorizontalBox> Badges = SNew(SHorizontalBox);
	FString FilterString = Handle->GetPropertyDisplayName().ToString();

	if (Property->HasMetaData(PlatformsMeta))
	{
		AddPlatformBadges(Property, *Badges, FilterString);
	}

	if (Property->HasMetaData(MinEngineMeta))
	{
		AddEngineBadge(Property, Row, *Badges, FilterString);
	}

	if (Property->HasMetaData(RequiresRebuildMeta))
	{
		AddRebuildBadge(*Badges, FilterString);
	}

	if (Property->HasMetaData(ExperimentalMeta))
	{
		AddExperimentalBadge(*Badges, FilterString);
	}

	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> ValueWidget;
	FDetailWidgetRow DefaultRow;
	Row.GetDefaultWidgets(NameWidget, ValueWidget, DefaultRow);

	// clang-format off
	Row.CustomWidget(true)
		.FilterString(FText::FromString(FilterString))
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				NameWidget.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				Badges
			]
		]
		.ValueContent()
		.MinDesiredWidth(DefaultRow.ValueWidget.MinWidth)
		.MaxDesiredWidth(DefaultRow.ValueWidget.MaxWidth)
		[
			ValueWidget.ToSharedRef()
		];
	// clang-format on
}

void FSentrySettingsBadges::AddPlatformBadges(const FProperty* Property, SHorizontalBox& Badges, FString& FilterString)
{
	TArray<FString> Tokens;
	Property->GetMetaData(PlatformsMeta).ParseIntoArray(Tokens, TEXT(","));

	// "!Platform" tokens list the only platforms where the setting has no effect
	const bool bExclusive = Tokens.ContainsByPredicate([](const FString& Token)
	{
		return Token.TrimStart().StartsWith(TEXT("!"));
	});

	for (FString& Token : Tokens)
	{
		Token.TrimStartAndEndInline();

		const bool bExcluded = Token.RemoveFromStart(TEXT("!"));
		if (bExcluded != bExclusive)
		{
			UE_LOG(LogSentrySettingsBadges, Warning, TEXT("SentryPlatforms of %s mixes included and excluded platforms. Token '%s' is ignored."), *Property->GetName(), *Token);
			continue;
		}

		FText Label, Name;
		if (!GetPlatformLabel(Token, Label, Name))
		{
			UE_LOG(LogSentrySettingsBadges, Warning, TEXT("SentryPlatforms of %s has an unknown platform '%s'."), *Property->GetName(), *Token);
			continue;
		}

		if (bExcluded)
		{
			AddBadge(Badges, FText::Format(FText::FromString(TEXT("NOT {0}")), Label),
				FText::Format(FText::FromString(TEXT("This setting has no effect on {0}. It has an effect on all other platforms.")), Name),
				PlatformBadgeColor, ExcludedPlatformBadgeTextColor);

			FilterString += TEXT(" not ") + Name.ToString();
		}
		else
		{
			AddBadge(Badges, Label,
				FText::Format(FText::FromString(TEXT("This setting has an effect on {0}.")), Name),
				PlatformBadgeColor, PlatformBadgeTextColor);

			FilterString += TEXT(" ") + Name.ToString();
		}
	}
}

void FSentrySettingsBadges::AddEngineBadge(const FProperty* Property, IDetailPropertyRow& Row, SHorizontalBox& Badges, FString& FilterString)
{
	const FString MinVersion = Property->GetMetaData(MinEngineMeta);

	FilterString += TEXT(" UE ") + MinVersion;

	if (IsEngineVersionSupported(MinVersion))
	{
		AddBadge(Badges, FText::FromString(FString::Printf(TEXT("UE %s+"), *MinVersion)),
			FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later.")), FText::FromString(MinVersion)),
			EngineBadgeColor, EngineBadgeTextColor);
	}
	else
	{
		AddBadge(Badges, FText::FromString(FString::Printf(TEXT("UE %s+"), *MinVersion)),
			FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later. It has no effect in this engine version.")), FText::FromString(MinVersion)),
			UnsupportedEngineBadgeColor, UnsupportedEngineBadgeTextColor);

		Row.IsEnabled(false);
	}
}

void FSentrySettingsBadges::AddRebuildBadge(SHorizontalBox& Badges, FString& FilterString)
{
	AddBadge(Badges, FText::FromString(TEXT("REBUILD")),
		FText::FromString(TEXT("The build reads this setting. Rebuild the project after you change it.")),
		RebuildBadgeColor, RebuildBadgeTextColor);

	FilterString += TEXT(" rebuild");
}

void FSentrySettingsBadges::AddExperimentalBadge(SHorizontalBox& Badges, FString& FilterString)
{
	AddBadge(Badges, FText::FromString(TEXT("EXPERIMENTAL")),
		FText::FromString(TEXT("This feature is experimental. Its behavior can change in a future release.")),
		ExperimentalBadgeColor, ExperimentalBadgeTextColor);

	FilterString += TEXT(" experimental");
}

void FSentrySettingsBadges::AddBadge(SHorizontalBox& Badges, const FText& Label, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor)
{
	Badges.AddSlot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
			[MakeBadge(Label, ToolTip, Color, TextColor)];
}

bool FSentrySettingsBadges::ShouldAddBadges(const FProperty* Property)
{
	return Property->HasMetaData(PlatformsMeta) ||
		   Property->HasMetaData(MinEngineMeta) ||
		   Property->HasMetaData(RequiresRebuildMeta) ||
		   Property->HasMetaData(ExperimentalMeta);
}

bool FSentrySettingsBadges::GetPlatformLabel(const FString& Token, FText& OutLabel, FText& OutName)
{
	if (Token == TEXT("Win64"))
	{
		OutLabel = FText::FromString(TEXT("WIN"));
		OutName = FText::FromString(TEXT("Windows"));
	}
	else if (Token == TEXT("Linux"))
	{
		OutLabel = FText::FromString(TEXT("LINUX"));
		OutName = FText::FromString(TEXT("Linux"));
	}
	else if (Token == TEXT("Mac"))
	{
		OutLabel = FText::FromString(TEXT("MAC"));
		OutName = FText::FromString(TEXT("macOS"));
	}
	else if (Token == TEXT("Mac:Cocoa"))
	{
		OutLabel = FText::FromString(TEXT("MAC (COCOA)"));
		OutName = FText::FromString(TEXT("macOS with the default (Cocoa) crash backend"));
	}
	else if (Token == TEXT("Mac:Native"))
	{
		OutLabel = FText::FromString(TEXT("MAC (NATIVE)"));
		OutName = FText::FromString(TEXT("macOS with the native crash backend"));
	}
	else if (Token == TEXT("IOS"))
	{
		OutLabel = FText::FromString(TEXT("IOS"));
		OutName = FText::FromString(TEXT("iOS"));
	}
	else if (Token == TEXT("Android"))
	{
		OutLabel = FText::FromString(TEXT("ANDROID"));
		OutName = FText::FromString(TEXT("Android"));
	}
	else if (Token == TEXT("Consoles"))
	{
		OutLabel = FText::FromString(TEXT("CONSOLES"));
		OutName = FText::FromString(TEXT("consoles"));
	}
	else if (Token == TEXT("Xbox"))
	{
		OutLabel = FText::FromString(TEXT("XBOX"));
		OutName = FText::FromString(TEXT("Xbox"));
	}
	else if (Token == TEXT("PS5"))
	{
		OutLabel = FText::FromString(TEXT("PS5"));
		OutName = FText::FromString(TEXT("PlayStation 5"));
	}
	else if (Token == TEXT("Switch"))
	{
		OutLabel = FText::FromString(TEXT("SWITCH"));
		OutName = FText::FromString(TEXT("Nintendo Switch"));
	}
	else
	{
		return false;
	}

	return true;
}

bool FSentrySettingsBadges::IsEngineVersionSupported(const FString& MinVersion)
{
	FString MajorStr, MinorStr;
	if (!MinVersion.Split(TEXT("."), &MajorStr, &MinorStr))
	{
		MajorStr = MinVersion;
	}

	const int32 Major = FCString::Atoi(*MajorStr);
	const int32 Minor = FCString::Atoi(*MinorStr);

	return ENGINE_MAJOR_VERSION > Major || (ENGINE_MAJOR_VERSION == Major && ENGINE_MINOR_VERSION >= Minor);
}

TSharedRef<SWidget> FSentrySettingsBadges::MakeBadge(const FText& Label, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor)
{
	// clang-format off
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush(TEXT("GenericWhiteBox")))
		.BorderBackgroundColor(Color)
		.Padding(FMargin(4.0f, 0.0f))
		.ToolTipText(ToolTip)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 7))
			.ColorAndOpacity(TextColor)
		];
	// clang-format on
}
