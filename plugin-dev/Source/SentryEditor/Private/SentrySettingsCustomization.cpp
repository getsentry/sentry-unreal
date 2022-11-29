// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettingsCustomization.h"
#include "SentrySettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "PropertyHandle.h"

#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Layout/SBorder.h"

#include "Launch/Resources/Version.h"
#include "Slate/Public/Widgets/Input/SButton.h"
#include "EditorStyle.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Styling/AppStyle.h"
#else
#include "EditorStyleSet.h"
#endif

const FString FSentrySettingsCustomization::DefaultCrcEndpoint = TEXT("https://datarouter.ol.epicgames.com/datarouter/api/v1/public/data");

void OnDocumentationLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata);

TSharedRef<IDetailCustomization> FSentrySettingsCustomization::MakeInstance()
{
	return MakeShareable(new FSentrySettingsCustomization);
}

void FSentrySettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DrawDebugSymbolsNotice(DetailBuilder);

	SetPropertiesUpdateHandler(DetailBuilder);
}

void FSentrySettingsCustomization::DrawDebugSymbolsNotice(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DebugSymbolsCategory = DetailBuilder.EditCategory(TEXT("Debug Symbols"));
	IDetailCategoryBuilder& CrashReporterCategory = DetailBuilder.EditCategory(TEXT("Crash Reporter"));

	TSharedPtr<IPropertyHandle> CrashReporterUrlHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USentrySettings, CrashReporterUrl));

#if ENGINE_MAJOR_VERSION >= 5
	const ISlateStyle& Style = FAppStyle::Get();
#else
	const ISlateStyle& Style = FEditorStyle::Get();
#endif

	DebugSymbolsCategory.AddCustomRow(FText::FromString(TEXT("DebugSymbols")), false)
		.WholeRowWidget
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
						.Text(FText::FromString(TEXT("Note that the Sentry SDK creates a <RichTextBlock.TextHighlight>sentry.properties</> file at project root to store the configuration, "
							"that should <RichTextBlock.TextHighlight>NOT</> be made publicly available.")))
						.TextStyle(Style, "MessageLog")
						.DecoratorStyleSet(&Style)
						.AutoWrapText(true)
				]
			]
		];

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
					.OnClicked_Lambda([=]() -> FReply
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
					.OnClicked_Lambda([=]() -> FReply
					{
						UpdateCrcConfig(DefaultCrcEndpoint);
						return FReply::Handled();
					})
					.Text(FText::FromString("Reset"))
					.ToolTipText(FText::FromString(TEXT("Reset crash reporter settings to defaults.")))
				]
			]
		];
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

FString FSentrySettingsCustomization::GetCrcConfigPath()
{
	return FPaths::Combine(FPaths::EngineDir(), TEXT("Programs"), TEXT("CrashReportClient"), TEXT("Config"), TEXT("DefaultEngine.ini"));
}

void OnDocumentationLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
{
	const FString* UrlPtr = Metadata.Find(TEXT("href"));
	if (UrlPtr)
	{
		FPlatformProcess::LaunchURL(**UrlPtr, nullptr, nullptr);
	}
}
