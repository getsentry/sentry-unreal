// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryCrashReporterImageryCustomization.h"
#include "SentrySettings.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "SExternalImageReference.h"

#include "Misc/Paths.h"
#include "PropertyHandle.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FSentryCrashReporterImageryCustomization::MakeInstance()
{
	return MakeShareable(new FSentryCrashReporterImageryCustomization);
}

void FSentryCrashReporterImageryCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> OverrideHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSentryCrashReporterImagery, bOverrideAppLogo));

	auto IsLogoPickerEnabled = [OverrideHandle]() -> bool
	{
		bool bEnabled = false;
		if (OverrideHandle.IsValid())
		{
			OverrideHandle->GetValue(bEnabled);
		}
		return bEnabled;
	};

	const FString TargetImagePath = GetCrashReporterLogoPath(TEXT("Logo.png"));
	TArray<FString> AllowedExtensions = { TEXT("png") };

	// clang-format off
	HeaderRow
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 4, 0))
			[
				OverrideHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				PropertyHandle->CreatePropertyNameWidget()
			]
		]
		.ValueContent()
		.MaxDesiredWidth(400.0f)
		.MinDesiredWidth(100.0f)
		[
			SNew(SBox)
			.IsEnabled_Lambda(IsLogoPickerEnabled)
			[
				SNew(SExternalImageReference, FString(), TargetImagePath)
				.FileDescription(FText::FromString(TEXT("crash reporter logo")))
				.FileExtensions(AllowedExtensions)
				.MaxDisplaySize(FVector2D(128.0f, 128.0f))
				.DeleteTargetWhenDefaultChosen(true)
			]
		];
	// clang-format on
}

void FSentryCrashReporterImageryCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// No children — everything is rendered in the header row
}

FString FSentryCrashReporterImageryCustomization::GetCrashReporterLogoPath(const FString& FileName) const
{
	return FPaths::Combine(FPaths::ProjectDir(), TEXT("Build"), TEXT("SentryCrashReporter"), FileName);
}
