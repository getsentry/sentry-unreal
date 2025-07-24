// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySettingsCustomization.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentrySymToolsDownloader.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
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
	DrawCrashReporterNotice(DetailBuilder);

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

void FSentrySettingsCustomization::DrawCrashReporterNotice(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CrashReporterCategory = DetailBuilder.EditCategory(TEXT("Crash Reporter"));

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
							.Text(FText::FromString(TEXT("In order to configure Crash Reporter use Sentry's Unreal Engine Endpoint from the Client Keys settings page. "
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
				.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"https://docs.sentry.io/platforms/unreal/setup-crashreporter/\">View the Crash Reporter setup documentation -></>")))
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
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if (Settings->Dsn.IsEmpty())
	{
		return static_cast<int32>(ESentrySettingsStatus::DsnMissing);
	}

	if (Settings->IsDirty() && SentrySubsystem->IsEnabled())
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
