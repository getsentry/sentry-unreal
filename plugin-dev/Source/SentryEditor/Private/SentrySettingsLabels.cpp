// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentrySettingsLabels.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogSentrySettingsLabels, Log, All);

const FName FSentrySettingsLabels::PlatformsMeta(TEXT("SentryPlatforms"));
const FName FSentrySettingsLabels::MinEngineMeta(TEXT("SentryMinEngine"));
const FName FSentrySettingsLabels::RequiresRebuildMeta(TEXT("SentryRequiresRebuild"));
const FName FSentrySettingsLabels::ExperimentalMeta(TEXT("SentryExperimental"));

const FLinearColor FSentrySettingsLabels::PlatformLabelColor(FColor(40, 46, 54));
const FLinearColor FSentrySettingsLabels::PlatformLabelTextColor(FColor(150, 160, 172));
const FLinearColor FSentrySettingsLabels::ExcludedPlatformLabelTextColor(FColor(214, 150, 140));
const FLinearColor FSentrySettingsLabels::EngineLabelColor(FColor(46, 42, 58));
const FLinearColor FSentrySettingsLabels::EngineLabelTextColor(FColor(165, 155, 188));
const FLinearColor FSentrySettingsLabels::UnsupportedEngineLabelColor(FColor(128, 34, 34));
const FLinearColor FSentrySettingsLabels::UnsupportedEngineLabelTextColor(FColor(255, 205, 205));
const FLinearColor FSentrySettingsLabels::RebuildLabelColor(FColor(122, 80, 18));
const FLinearColor FSentrySettingsLabels::RebuildLabelTextColor(FColor(255, 218, 160));
const FLinearColor FSentrySettingsLabels::ExperimentalLabelColor(FColor(24, 66, 72));
const FLinearColor FSentrySettingsLabels::ExperimentalLabelTextColor(FColor(150, 220, 226));

void FSentrySettingsLabels::AddLabels(IDetailLayoutBuilder& DetailBuilder)
{
	for (TFieldIterator<FProperty> It(USentrySettings::StaticClass()); It; ++It)
	{
		const FProperty* Property = *It;
		if (!ShouldAddLabels(Property))
		{
			continue;
		}

		TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Property->GetFName());
		if (IDetailPropertyRow* Row = DetailBuilder.EditDefaultProperty(Handle))
		{
			AddLabels(*Row, Handle, Property);
		}
	}
}

void FSentrySettingsLabels::AddLabels(IDetailPropertyRow& Row, TSharedRef<IPropertyHandle> Handle, const FProperty* Property)
{
	TSharedRef<SHorizontalBox> Labels = SNew(SHorizontalBox);
	FString FilterString = Handle->GetPropertyDisplayName().ToString();

	if (Property->HasMetaData(PlatformsMeta))
	{
		AddPlatformLabels(Property, *Labels, FilterString);
	}

	if (Property->HasMetaData(MinEngineMeta))
	{
		AddEngineLabel(Property, Row, *Labels, FilterString);
	}

	if (Property->HasMetaData(RequiresRebuildMeta))
	{
		AddRebuildLabel(*Labels, FilterString);
	}

	if (Property->HasMetaData(ExperimentalMeta))
	{
		AddExperimentalLabel(*Labels, FilterString);
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
				Labels
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

void FSentrySettingsLabels::AddPlatformLabels(const FProperty* Property, SHorizontalBox& Labels, FString& FilterString)
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
			UE_LOG(LogSentrySettingsLabels, Warning, TEXT("SentryPlatforms of %s mixes included and excluded platforms. Token '%s' is ignored."), *Property->GetName(), *Token);
			continue;
		}

		FText ShortName, Name;
		if (!GetPlatformNames(Token, ShortName, Name))
		{
			UE_LOG(LogSentrySettingsLabels, Warning, TEXT("SentryPlatforms of %s has an unknown platform '%s'."), *Property->GetName(), *Token);
			continue;
		}

		if (bExcluded)
		{
			AddLabel(Labels, FText::Format(FText::FromString(TEXT("NOT {0}")), ShortName),
				FText::Format(FText::FromString(TEXT("This setting has no effect on {0}. It has an effect on all other platforms.")), Name),
				PlatformLabelColor, ExcludedPlatformLabelTextColor);

			FilterString += TEXT(" not ") + Name.ToString();
		}
		else
		{
			AddLabel(Labels, ShortName,
				FText::Format(FText::FromString(TEXT("This setting has an effect on {0}.")), Name),
				PlatformLabelColor, PlatformLabelTextColor);

			FilterString += TEXT(" ") + Name.ToString();
		}
	}
}

void FSentrySettingsLabels::AddEngineLabel(const FProperty* Property, IDetailPropertyRow& Row, SHorizontalBox& Labels, FString& FilterString)
{
	const FString MinVersion = Property->GetMetaData(MinEngineMeta);

	FilterString += TEXT(" UE ") + MinVersion;

	if (IsEngineVersionSupported(MinVersion))
	{
		AddLabel(Labels, FText::FromString(FString::Printf(TEXT("UE %s+"), *MinVersion)),
			FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later.")), FText::FromString(MinVersion)),
			EngineLabelColor, EngineLabelTextColor);
	}
	else
	{
		AddLabel(Labels, FText::FromString(FString::Printf(TEXT("UE %s+"), *MinVersion)),
			FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later. It has no effect in this engine version.")), FText::FromString(MinVersion)),
			UnsupportedEngineLabelColor, UnsupportedEngineLabelTextColor);

		Row.IsEnabled(false);
	}
}

void FSentrySettingsLabels::AddRebuildLabel(SHorizontalBox& Labels, FString& FilterString)
{
	AddLabel(Labels, FText::FromString(TEXT("REBUILD")),
		FText::FromString(TEXT("The build reads this setting. Rebuild the project after you change it.")),
		RebuildLabelColor, RebuildLabelTextColor);

	FilterString += TEXT(" rebuild");
}

void FSentrySettingsLabels::AddExperimentalLabel(SHorizontalBox& Labels, FString& FilterString)
{
	AddLabel(Labels, FText::FromString(TEXT("EXPERIMENTAL")),
		FText::FromString(TEXT("This feature is experimental. Its behavior can change in a future release.")),
		ExperimentalLabelColor, ExperimentalLabelTextColor);

	FilterString += TEXT(" experimental");
}

void FSentrySettingsLabels::AddLabel(SHorizontalBox& Labels, const FText& Text, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor)
{
	Labels.AddSlot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
			[MakeLabel(Text, ToolTip, Color, TextColor)];
}

bool FSentrySettingsLabels::ShouldAddLabels(const FProperty* Property)
{
	return Property->HasMetaData(PlatformsMeta) ||
		   Property->HasMetaData(MinEngineMeta) ||
		   Property->HasMetaData(RequiresRebuildMeta) ||
		   Property->HasMetaData(ExperimentalMeta);
}

bool FSentrySettingsLabels::GetPlatformNames(const FString& Token, FText& OutShortName, FText& OutName)
{
	if (Token == TEXT("Win64"))
	{
		OutShortName = FText::FromString(TEXT("WIN"));
		OutName = FText::FromString(TEXT("Windows"));
	}
	else if (Token == TEXT("Linux"))
	{
		OutShortName = FText::FromString(TEXT("LINUX"));
		OutName = FText::FromString(TEXT("Linux"));
	}
	else if (Token == TEXT("Mac"))
	{
		OutShortName = FText::FromString(TEXT("MAC"));
		OutName = FText::FromString(TEXT("macOS"));
	}
	else if (Token == TEXT("Mac:Cocoa"))
	{
		OutShortName = FText::FromString(TEXT("MAC (COCOA)"));
		OutName = FText::FromString(TEXT("macOS with the default (Cocoa) crash backend"));
	}
	else if (Token == TEXT("Mac:Native"))
	{
		OutShortName = FText::FromString(TEXT("MAC (NATIVE)"));
		OutName = FText::FromString(TEXT("macOS with the native crash backend"));
	}
	else if (Token == TEXT("IOS"))
	{
		OutShortName = FText::FromString(TEXT("IOS"));
		OutName = FText::FromString(TEXT("iOS"));
	}
	else if (Token == TEXT("Android"))
	{
		OutShortName = FText::FromString(TEXT("ANDROID"));
		OutName = FText::FromString(TEXT("Android"));
	}
	else if (Token == TEXT("Consoles"))
	{
		OutShortName = FText::FromString(TEXT("CONSOLES"));
		OutName = FText::FromString(TEXT("consoles"));
	}
	else if (Token == TEXT("Xbox"))
	{
		OutShortName = FText::FromString(TEXT("XBOX"));
		OutName = FText::FromString(TEXT("Xbox"));
	}
	else if (Token == TEXT("PS5"))
	{
		OutShortName = FText::FromString(TEXT("PS5"));
		OutName = FText::FromString(TEXT("PlayStation 5"));
	}
	else if (Token == TEXT("Switch"))
	{
		OutShortName = FText::FromString(TEXT("SWITCH"));
		OutName = FText::FromString(TEXT("Nintendo Switch"));
	}
	else
	{
		return false;
	}

	return true;
}

bool FSentrySettingsLabels::IsEngineVersionSupported(const FString& MinVersion)
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

TSharedRef<SWidget> FSentrySettingsLabels::MakeLabel(const FText& Text, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor)
{
	// clang-format off
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush(TEXT("GenericWhiteBox")))
		.BorderBackgroundColor(Color)
		.Padding(FMargin(4.0f, 0.0f))
		.ToolTipText(ToolTip)
		[
			SNew(STextBlock)
			.Text(Text)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 7))
			.ColorAndOpacity(TextColor)
		];
	// clang-format on
}
