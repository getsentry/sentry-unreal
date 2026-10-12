// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySettingsCustomization.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentrySymToolsDownloader.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "IUATHelperModule.h"

#include "Engine/Engine.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"
#include "PropertyHandle.h"

#include "Interfaces/IPluginManager.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#if UE_VERSION_OLDER_THAN(5, 0, 0)
#include "EditorStyleSet.h"
#else
#include "Styling/AppStyle.h"
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
#include "HAL/PlatformFilemanager.h"
#else
#include "HAL/PlatformFileManager.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSentryEditorSettings, Log, All);

const FString FSentrySettingsCustomization::DefaultCrcEndpoint = TEXT("https://datarouter.ol.epicgames.com/datarouter/api/v1/public/data");

void OnDocumentationLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata);

FSentrySettingsCustomization::FSentrySettingsCustomization()
	: CliDownloader(MakeShareable(new FSentrySymToolsDownloader()))
	, IsCompilingLinuxBinaries(false)
{
}

FSentrySettingsCustomization::~FSentrySettingsCustomization()
{
}

TSharedRef<IDetailCustomization> FSentrySettingsCustomization::MakeInstance()
{
	return MakeShareable(new FSentrySettingsCustomization);
}

void FSentrySettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DrawGeneralNotice(DetailBuilder);
	DrawDebugSymbolsNotice(DetailBuilder);
	DrawSentryCrashReporterSection(DetailBuilder);
	DrawUnrealCrashReporterNotice(DetailBuilder);
	DrawSettingBadges(DetailBuilder);

	SetPropertiesUpdateHandler(DetailBuilder);
}

void FSentrySettingsCustomization::DrawGeneralNotice(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& GeneralCategory = DetailBuilder.EditCategory(TEXT("General"));

	TSharedRef<SWidget> GeneralMissingDsnWidget = MakeGeneralSettingsStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
		FText::FromString(TEXT("Sentry DSN is not configured.")), FText());

	TSharedRef<SWidget> GeneralModifiedWidget = MakeGeneralSettingsStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
		FText::FromString(TEXT("Sentry settings were modified.")), FText::FromString(TEXT("Apply")));

	TSharedRef<SWidget> GeneralConfiguredWidget = MakeGeneralSettingsStatusRow(FName(TEXT("SettingsEditor.GoodIcon")),
		FText::FromString(TEXT("Sentry is configured.")), FText());

	// clang-format off
	GeneralCategory.AddCustomRow(FText::FromString(TEXT("General")), false)
		.WholeRowWidget
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex(this, &FSentrySettingsCustomization::GetGeneralSettingsStatusAsInt)
				+SWidgetSwitcher::Slot()
				[
					GeneralMissingDsnWidget
				]
				+SWidgetSwitcher::Slot()
				[
					GeneralModifiedWidget
				]
				+SWidgetSwitcher::Slot()
				[
					GeneralConfiguredWidget
				]
			]
		];
	// clang-format on

#if PLATFORM_WINDOWS
	if (FSentryModule::Get().IsMarketplaceVersion())
	{
		TSharedRef<SWidget> LinuxBinariesMissingWidget = MakeLinuxBinariesStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
			FText::FromString(TEXT("Sentry Linux pre-compiled binaries are missing.")), FText::FromString(TEXT("Compile")));

		TSharedRef<SWidget> LinuxBinariesCompilingWidget = MakeLinuxBinariesStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
			FText::FromString(TEXT("Compiling Sentry for Linux...")), FText());

		TSharedRef<SWidget> LinuxBinariesConfiguredWidget = MakeLinuxBinariesStatusRow(FName(TEXT("SettingsEditor.GoodIcon")),
			FText::FromString(TEXT("Sentry Linux pre-compiled binaries are ready.")), FText());

		// clang-format off
		GeneralCategory.AddCustomRow(FText::FromString(TEXT("General")), false)
			.WholeRowWidget
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(this, &FSentrySettingsCustomization::GetLinuxBinariesStatusAsInt)
					+SWidgetSwitcher::Slot()
					[
						LinuxBinariesMissingWidget
					]
					+SWidgetSwitcher::Slot()
					[
						LinuxBinariesCompilingWidget
					]
					+SWidgetSwitcher::Slot()
					[
						LinuxBinariesConfiguredWidget
					]
				]
			];
		// clang-format on
	}
#endif
}

void FSentrySettingsCustomization::DrawSentryCrashReporterSection(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& SentryCrashReporterCategory = DetailBuilder.EditCategory(TEXT("Sentry Crash Reporter"));

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const ISlateStyle& Style = FEditorStyle::Get();
#else
	const ISlateStyle& Style = FAppStyle::Get();
#endif

	// clang-format off
	SentryCrashReporterCategory.AddCustomRow(FText::FromString(TEXT("SentryCrashReporter")), false)
		.WholeRowWidget
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(1)
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(10, 10, 10, 10))
					.FillWidth(1.0f)
					[
						SNew(SRichTextBlock)
							.Text(FText::FromString(TEXT("Sentry Crash Reporter is a standalone application that can be used instead of the default Unreal Crash Reporter. "
								"It lets users review crash details (tags, context, stacktrace) and submit feedback before the report is sent to Sentry. "
								"Supported on all desktop platforms (on macOS the <RichTextBlock.TextHighlight>native crash backend</> must be enabled). "
								"For symbolicated stacktraces in Shipping builds, enable \"Include Debug Files in Shipping Builds\" in packaging settings.")))
							.TextStyle(Style, "MessageLog")
							.DecoratorStyleSet(&Style)
							.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 0, 10))
			.VAlign(VAlign_Top)
			[
				SNew(SRichTextBlock)
				.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"https://docs.sentry.io/platforms/unreal/configuration/crash-reporter/sentry-crash-reporter/#sentry-crash-reporter\">View the Sentry Crash Reporter setup documentation -></>")))
				.AutoWrapText(true)
				.DecoratorStyleSet(&FCoreStyle::Get())
				+ SRichTextBlock::HyperlinkDecorator(TEXT("browser"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnDocumentationLinkClicked))
			]
		];
	// clang-format on
}

void FSentrySettingsCustomization::DrawUnrealCrashReporterNotice(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CrashReporterCategory = DetailBuilder.EditCategory(TEXT("Unreal Crash Reporter"));

	TSharedPtr<IPropertyHandle> CrashReporterUrlHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USentrySettings, CrashReporterUrl));

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const ISlateStyle& Style = FEditorStyle::Get();
#else
	const ISlateStyle& Style = FAppStyle::Get();
#endif

	// clang-format off
	CrashReporterCategory.AddCustomRow(FText::FromString(TEXT("CrashReporter")), false)
		.WholeRowWidget
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(1)
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(10, 10, 10, 10))
					.FillWidth(1.0f)
					[
						SNew(SRichTextBlock)
							.Text(FText::FromString(TEXT("In order to configure Unreal Crash Reporter use Sentry's Unreal Engine Endpoint from the Client Keys settings page. "
								"This will include which project within Sentry you want to see the crashes arriving in real time. "
								"Note that it's accomplished by modifying the `CrashReportClient` section in the global <RichTextBlock.TextHighlight>DefaultEngine.ini</> file. "
								"Changing the engine is necessary for this to work!")))
							.TextStyle(Style, "MessageLog")
							.DecoratorStyleSet(&Style)
							.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 0, 10))
			.VAlign(VAlign_Top)
			[
				SNew(SRichTextBlock)
				.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"https://docs.sentry.io/platforms/unreal/configuration/crash-reporter/crash-reporter-client/\">View the Unreal Crash Reporter setup documentation -></>")))
				.AutoWrapText(true)
				.DecoratorStyleSet(&FCoreStyle::Get())
				+ SRichTextBlock::HyperlinkDecorator(TEXT("browser"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnDocumentationLinkClicked))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 0, 10))
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0, 0, 5, 0))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Lambda([this, CrashReporterUrlHandle]() -> FReply
					{
						FString CrcEndpoint;
						CrashReporterUrlHandle->GetValue(CrcEndpoint);
						UpdateCrcConfig(CrcEndpoint);
						return FReply::Handled();
					})
					.Text(FText::FromString(TEXT("Update global settings")))
					.ToolTipText(FText::FromString(TEXT("Update global crash reporter settings in DefaultEngine.ini configuration file.")))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Lambda([this]() -> FReply
					{
						UpdateCrcConfig(DefaultCrcEndpoint);
						return FReply::Handled();
					})
					.Text(FText::FromString("Reset"))
					.ToolTipText(FText::FromString(TEXT("Reset crash reporter settings to defaults.")))
				]
			]
		];
	// clang-format on
}

void FSentrySettingsCustomization::DrawDebugSymbolsNotice(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DebugSymbolsCategory = DetailBuilder.EditCategory(TEXT("Debug Symbols"));

	TSharedRef<SWidget> CliMissingWidget = MakeSentryCliStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
		FText::FromString(TEXT("Sentry symbol upload tools are not configured.")), FText::FromString(TEXT("Configure Now")));

	TSharedRef<SWidget> CliDownloadingWidget = MakeSentryCliStatusRow(FName(TEXT("SettingsEditor.WarningIcon")),
		FText::FromString(TEXT("Downloading Sentry symbol upload tools...")), FText());

	TSharedRef<SWidget> CliConfiguredWidget = MakeSentryCliStatusRow(FName(TEXT("SettingsEditor.GoodIcon")),
		FText::FromString(TEXT("Sentry symbol upload tools are configured.")), FText::FromString(TEXT("Reload")));

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const ISlateStyle& Style = FEditorStyle::Get();
#else
	const ISlateStyle& Style = FAppStyle::Get();
#endif

	// clang-format off
	DebugSymbolsCategory.AddCustomRow(FText::FromString(TEXT("DebugSymbols")), false)
		.WholeRowWidget
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex(this, &FSentrySettingsCustomization::GetSentryCliStatusAsInt)
				+SWidgetSwitcher::Slot()
				[
					CliMissingWidget
				]
				+SWidgetSwitcher::Slot()
				[
					CliDownloadingWidget
				]
				+SWidgetSwitcher::Slot()
				[
					CliConfiguredWidget
				]
			]
		];
	// clang-format on

	// clang-format off
	DebugSymbolsCategory.AddCustomRow(FText::FromString(TEXT("DebugSymbols")), false)
		.WholeRowWidget
		[
			SNew(SBorder)
			.Padding(1)
			[
				SNew(SBorder)
				.Padding(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(10, 10, 10, 10))
					.FillWidth(1.0f)
					[
						SNew(SRichTextBlock)
							.Text(FText::FromString(TEXT("Note that the Sentry SDK automatically creates a <RichTextBlock.TextHighlight>sentry.properties</> file at project root to store the configuration, "
								"that should <RichTextBlock.TextHighlight>NOT</> be made publicly available. Alternatively, set up the environment variables "
								"'SENTRY_PROJECT', 'SENTRY_ORG' and 'SENTRY_AUTH_TOKEN' with the necessary information to upload debug symbols on the build agent.")))
							.TextStyle(Style, "MessageLog")
							.DecoratorStyleSet(&Style)
							.AutoWrapText(true)
					]
				]
			]
		];
	// clang-format on
}

namespace SentrySettingBadges
{
const FName PlatformsMeta(TEXT("SentryPlatforms"));
const FName MinEngineMeta(TEXT("SentryMinEngine"));
const FName RequiresRebuildMeta(TEXT("SentryRequiresRebuild"));
const FName ExperimentalMeta(TEXT("SentryExperimental"));

const FLinearColor PlatformColor(FColor(40, 46, 54));
const FLinearColor PlatformTextColor(FColor(150, 160, 172));
const FLinearColor ExcludedTextColor(FColor(214, 150, 140));
const FLinearColor EngineColor(FColor(46, 42, 58));
const FLinearColor EngineTextColor(FColor(165, 155, 188));
const FLinearColor RebuildColor(FColor(122, 80, 18));
const FLinearColor RebuildTextColor(FColor(255, 218, 160));
const FLinearColor ExperimentalColor(FColor(24, 66, 72));
const FLinearColor ExperimentalTextColor(FColor(150, 220, 226));
const FLinearColor EngineUnsupportedColor(FColor(128, 34, 34));
const FLinearColor EngineUnsupportedTextColor(FColor(255, 205, 205));

bool GetPlatformLabel(const FString& Token, FText& OutLabel, FText& OutName)
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

bool IsEngineVersionSupported(const FString& MinVersion)
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
} // namespace SentrySettingBadges

void FSentrySettingsCustomization::DrawSettingBadges(IDetailLayoutBuilder& DetailBuilder)
{
	using namespace SentrySettingBadges;

	for (TFieldIterator<FProperty> It(USentrySettings::StaticClass()); It; ++It)
	{
		const FProperty* Property = *It;

		const bool bHasPlatforms = Property->HasMetaData(PlatformsMeta);
		const bool bHasMinEngine = Property->HasMetaData(MinEngineMeta);
		const bool bRequiresRebuild = Property->HasMetaData(RequiresRebuildMeta);
		const bool bExperimental = Property->HasMetaData(ExperimentalMeta);

		if (!bHasPlatforms && !bHasMinEngine && !bRequiresRebuild && !bExperimental)
		{
			continue;
		}

		TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(Property->GetFName());
		IDetailPropertyRow* Row = DetailBuilder.EditDefaultProperty(Handle);
		if (!Row)
		{
			continue;
		}

		TSharedRef<SHorizontalBox> Badges = SNew(SHorizontalBox);
		FString FilterString = Handle->GetPropertyDisplayName().ToString();

		if (bHasPlatforms)
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
					UE_LOG(LogSentryEditorSettings, Warning, TEXT("SentryPlatforms of %s mixes included and excluded platforms. Token '%s' is ignored."), *Property->GetName(), *Token);
					continue;
				}

				FText Label, Name;
				if (!GetPlatformLabel(Token, Label, Name))
				{
					UE_LOG(LogSentryEditorSettings, Warning, TEXT("SentryPlatforms of %s has an unknown platform '%s'."), *Property->GetName(), *Token);
					continue;
				}

				if (bExcluded)
				{
					Badges->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 3.0f, 0.0f)
						[MakeBadge(FText::Format(FText::FromString(TEXT("NOT {0}")), Label),
							FText::Format(FText::FromString(TEXT("This setting has no effect on {0}. It has an effect on all other platforms.")), Name),
							PlatformColor, ExcludedTextColor)];

					FilterString += TEXT(" not ") + Name.ToString();
				}
				else
				{
					Badges->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 3.0f, 0.0f)
						[MakeBadge(Label, FText::Format(FText::FromString(TEXT("This setting has an effect on {0}.")), Name), PlatformColor, PlatformTextColor)];

					FilterString += TEXT(" ") + Name.ToString();
				}
			}
		}

		if (bHasMinEngine)
		{
			const FString MinVersion = Property->GetMetaData(MinEngineMeta);
			const bool bSupported = IsEngineVersionSupported(MinVersion);

			Badges->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 3.0f, 0.0f)
				[MakeBadge(FText::FromString(FString::Printf(TEXT("UE %s+"), *MinVersion)),
					bSupported
						? FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later.")), FText::FromString(MinVersion))
						: FText::Format(FText::FromString(TEXT("This setting needs Unreal Engine {0} or later. It has no effect in this engine version.")), FText::FromString(MinVersion)),
					bSupported ? EngineColor : EngineUnsupportedColor,
					bSupported ? EngineTextColor : EngineUnsupportedTextColor)];

			if (!bSupported)
			{
				Row->IsEnabled(false);
			}
		}

		if (bRequiresRebuild)
		{
			Badges->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 3.0f, 0.0f)
				[MakeBadge(FText::FromString(TEXT("REBUILD")),
					FText::FromString(TEXT("The build reads this setting. Rebuild the project after you change it.")),
					RebuildColor, RebuildTextColor)];

			FilterString += TEXT(" rebuild");
		}

		if (bExperimental)
		{
			Badges->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 3.0f, 0.0f)
				[MakeBadge(FText::FromString(TEXT("EXPERIMENTAL")),
					FText::FromString(TEXT("This feature is experimental. Its behavior can change in a future release.")),
					ExperimentalColor, ExperimentalTextColor)];

			FilterString += TEXT(" experimental");
		}

		TSharedPtr<SWidget> NameWidget;
		TSharedPtr<SWidget> ValueWidget;
		FDetailWidgetRow DefaultRow;
		Row->GetDefaultWidgets(NameWidget, ValueWidget, DefaultRow);

		// clang-format off
		Row->CustomWidget(true)
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
}

TSharedRef<SWidget> FSentrySettingsCustomization::MakeBadge(const FText& Label, const FText& ToolTip, const FLinearColor& Color, const FLinearColor& TextColor)
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

void FSentrySettingsCustomization::SetPropertiesUpdateHandler(IDetailLayoutBuilder& DetailBuilder)
{
	const FSimpleDelegate OnUpdateProjectName = FSimpleDelegate::CreateSP(this, &FSentrySettingsCustomization::UpdateProjectName);
	ProjectNameHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USentrySettings, ProjectName));
	ProjectNameHandle->SetOnPropertyValueChanged(OnUpdateProjectName);

	const FSimpleDelegate OnUpdateOrganizationName = FSimpleDelegate::CreateSP(this, &FSentrySettingsCustomization::UpdateOrganizationName);
	OrganizationNameHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USentrySettings, OrgName));
	OrganizationNameHandle->SetOnPropertyValueChanged(OnUpdateOrganizationName);

	const FSimpleDelegate OnUpdateAuthToken = FSimpleDelegate::CreateSP(this, &FSentrySettingsCustomization::UpdateAuthToken);
	AuthTokenHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USentrySettings, AuthToken));
	AuthTokenHandle->SetOnPropertyValueChanged(OnUpdateAuthToken);
}

TSharedRef<SWidget> FSentrySettingsCustomization::MakeGeneralSettingsStatusRow(FName IconName, FText Message, FText ButtonMessage)
{
	// clang-format off
	TSharedRef<SHorizontalBox> Result = SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
	#if UE_VERSION_OLDER_THAN(5, 0, 0)
			.Image(FEditorStyle::Get().GetBrush(IconName))
	#else
			.Image(FAppStyle::Get().GetBrush(IconName))
	#endif
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(16.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FLinearColor::White)
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FVector2D::UnitVector)
			.Text(Message)
		];
	// clang-format on

	if (!ButtonMessage.IsEmpty())
	{
		// clang-format off
		Result->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.OnClicked_Lambda([this]() -> FReply
				{
					USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
					SentrySubsystem->Close();
					SentrySubsystem->Initialize();

					USentrySettings* Settings = FSentryModule::Get().GetSettings();
					Settings->ClearDirtyFlag();

					return FReply::Handled();
				})
				.Text(ButtonMessage)
			];
		// clang-format on
	}

	return Result;
}

TSharedRef<SWidget> FSentrySettingsCustomization::MakeLinuxBinariesStatusRow(FName IconName, FText Message, FText ButtonMessage)
{
	// clang-format off
	TSharedRef<SHorizontalBox> Result = SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
	#if UE_VERSION_OLDER_THAN(5, 0, 0)
			.Image(FEditorStyle::Get().GetBrush(IconName))
	#else
			.Image(FAppStyle::Get().GetBrush(IconName))
	#endif
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(16.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FLinearColor::White)
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FVector2D::UnitVector)
			.Text(Message)
		];
	// clang-format on

	if (!ButtonMessage.IsEmpty())
	{
		// clang-format off
		Result->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.OnClicked_Lambda([this]() -> FReply
				{
					IsCompilingLinuxBinaries = true;

					// In case the plugin installed via Epic Games launcher it's supposed to be <EngineDir>/Plugins/Marketplace/Sentry
					const FString PluginPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir());

					const FString TempLinuxBinariesPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Intermediate"), TEXT("SentryLinuxBinaries")));

					FString CommandLine = FString::Printf(TEXT("BuildPlugin -Plugin=\"%s/Sentry.uplugin\" -Package=\"%s\" -CreateSubFolder -TargetPlatforms=Linux"), *PluginPath, *TempLinuxBinariesPath);

					IUATHelperModule::Get().CreateUatTask(CommandLine, FText::FromString("Windows"),
						FText::FromString("Compiling Sentry for Linux"),
						FText::FromString("Compile Sentry Linux"),
					#if UE_VERSION_OLDER_THAN(5, 1, 0)
						FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
					#else
						FAppStyle::GetBrush(TEXT("MainFrame.CookContent")),
						nullptr,
					#endif
						[this, TempLinuxBinariesPath](FString result, double X)
						{
							if (result.Equals(TEXT("Completed")))
							{
								FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*GetLinuxBinariesDirPath(),
									*FPaths::Combine(TempLinuxBinariesPath, TEXT("Intermediate"), TEXT("Build"), TEXT("Linux")), true);
							}

							IsCompilingLinuxBinaries = false;
						});

					return FReply::Handled();
				})
				.Text(ButtonMessage)
			];
		// clang-format on
	}

	return Result;
}

TSharedRef<SWidget> FSentrySettingsCustomization::MakeSentryCliStatusRow(FName IconName, FText Message, FText ButtonMessage)
{
	// clang-format off
	TSharedRef<SHorizontalBox> Result = SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			.Image(FEditorStyle::Get().GetBrush(IconName))
#else
			.Image(FAppStyle::Get().GetBrush(IconName))
#endif
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(16.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FLinearColor::White)
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FVector2D::UnitVector)
			.Text(Message)
		];
	// clang-format on

	if (!ButtonMessage.IsEmpty())
	{
		// clang-format off
		Result->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.OnClicked_Lambda([this]() -> FReply
				{
					if(CliDownloader.IsValid() && CliDownloader->GetStatus() != ESentrySymToolsStatus::Downloading)
					{
						CliDownloader->Download([](bool Result)
						{
							FNotificationInfo Info(FText::FromString(Result
								? TEXT("Sentry symbol upload tools was configured successfully.")
								: TEXT("Sentry symbol upload tools configuration failed.")));
							Info.ExpireDuration = 3.0f;
							Info.bUseSuccessFailIcons = true;

							TSharedPtr<SNotificationItem> EditorNotification = FSlateNotificationManager::Get().AddNotification(Info);
							EditorNotification->SetCompletionState(Result ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
						});
					}

					return FReply::Handled();
				})
				.Text(ButtonMessage)
			];
		// clang-format on
	}

	return Result;
}

void FSentrySettingsCustomization::UpdateProjectName()
{
	FString Value;
	ProjectNameHandle->GetValue(Value);

	UpdatePropertiesFile(TEXT("defaults.project"), Value);
}

void FSentrySettingsCustomization::UpdateOrganizationName()
{
	FString Value;
	OrganizationNameHandle->GetValue(Value);

	UpdatePropertiesFile(TEXT("defaults.org"), Value);
}

void FSentrySettingsCustomization::UpdateAuthToken()
{
	FString Value;
	AuthTokenHandle->GetValue(Value);

	UpdatePropertiesFile(TEXT("auth.token"), Value);
}

void FSentrySettingsCustomization::UpdatePropertiesFile(const FString& PropertyName, const FString& PropertyValue)
{
	const FString PropertiesFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("sentry.properties"));

	FConfigFile PropertiesFile;

	if (FPaths::FileExists(PropertiesFilePath))
	{
		PropertiesFile.Read(PropertiesFilePath);
	}

	PropertiesFile.SetString(TEXT("Sentry"), *PropertyName, *PropertyValue);
	PropertiesFile.Write(PropertiesFilePath);
}

void FSentrySettingsCustomization::UpdateCrcConfig(const FString& Url)
{
	if (Url.IsEmpty())
	{
		return;
	}

	const FString CrcConfigFilePath = GetCrcConfigPath();

	if (!FPaths::FileExists(CrcConfigFilePath))
	{
		return;
	}

	FConfigCacheIni CrcConfigFile(EConfigCacheType::DiskBacked);
	CrcConfigFile.LoadFile(CrcConfigFilePath);

	const FString CrcSectionName = FString(TEXT("CrashReportClient"));

	const FString DataRouterUrlKey = FString(TEXT("DataRouterUrl"));
	const FString DataRouterUrlValue = Url;

	CrcConfigFile.SetString(*CrcSectionName, *DataRouterUrlKey, *DataRouterUrlValue, CrcConfigFilePath);
}

FString FSentrySettingsCustomization::GetCrcConfigPath() const
{
	return FPaths::Combine(FPaths::EngineDir(), TEXT("Programs"), TEXT("CrashReportClient"), TEXT("Config"), TEXT("DefaultEngine.ini"));
}

FString FSentrySettingsCustomization::GetLinuxBinariesDirPath() const
{
	const FString PluginPath = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	return FPaths::Combine(PluginPath, TEXT("Intermediate"), TEXT("Build"), TEXT("Linux"));
}

int32 FSentrySettingsCustomization::GetGeneralSettingsStatusAsInt() const
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if (Settings->Dsn.IsEmpty())
	{
		return static_cast<int32>(ESentrySettingsStatus::DsnMissing);
	}

	if (Settings->IsDirty())
	{
		return static_cast<int32>(ESentrySettingsStatus::Modified);
	}

	return static_cast<int32>(ESentrySettingsStatus::Configured);
}

int32 FSentrySettingsCustomization::GetLinuxBinariesStatusAsInt() const
{
	if (IsCompilingLinuxBinaries)
	{
		return static_cast<int32>(ESentryLinuxBinariesStatus::Compiling);
	}

	if (IFileManager::Get().DirectoryExists(*GetLinuxBinariesDirPath()))
	{
		return static_cast<int32>(ESentryLinuxBinariesStatus::Configured);
	}

	return static_cast<int32>(ESentryLinuxBinariesStatus::Missing);
}

int32 FSentrySettingsCustomization::GetSentryCliStatusAsInt() const
{
	if (CliDownloader.IsValid())
	{
		return static_cast<int32>(CliDownloader->GetStatus());
	}

	return 0;
}

void OnDocumentationLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
{
	const FString* UrlPtr = Metadata.Find(TEXT("href"));
	if (UrlPtr)
	{
		FPlatformProcess::LaunchURL(**UrlPtr, nullptr, nullptr);
	}
}
