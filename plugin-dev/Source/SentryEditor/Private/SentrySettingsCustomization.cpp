// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettingsCustomization.h"
#include "SentrySettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorStyleSet.h"
#include "Misc/Paths.h"

#include "Widgets/Text/SRichTextBlock.h"

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
						.Text(FText::FromString(TEXT("Note that the Sentry SDK creates a <RichTextBlock.TextHighlight>sentry.properties</> file at project root to store the configuration, that should <RichTextBlock.TextHighlight>NOT</> be made publicly available.")))
						.TextStyle(FEditorStyle::Get(), "MessageLog")
						.DecoratorStyleSet(&FEditorStyle::Get())
						.AutoWrapText(true)
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
