// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryCrashReporterViewCustomization.h"
#include "SentrySettings.h"

#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyHandle.h"

#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FSentryCrashReporterViewCustomization::MakeInstance()
{
	return MakeShareable(new FSentryCrashReporterViewCustomization);
}

void FSentryCrashReporterViewCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ViewHandle = PropertyHandle;

	// clang-format off
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(150.0f)
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &FSentryCrashReporterViewCustomization::MakeViewMenu)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &FSentryCrashReporterViewCustomization::GetCurrentViewText)
				.Font(CustomizationUtils.GetRegularFont())
			]
		];
	// clang-format on
}

void FSentryCrashReporterViewCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

TSharedRef<SWidget> FSentryCrashReporterViewCustomization::MakeViewMenu() const
{
	const UEnum* Enum = StaticEnum<ESentryCrashReporterView>();
	const TSet<uint8> UsedViews = GetViewsUsedBySiblings();

	FMenuBuilder MenuBuilder(true, nullptr);

	for (int32 Index = 0; Index < Enum->NumEnums() - 1; ++Index)
	{
		const uint8 Value = static_cast<uint8>(Enum->GetValueByIndex(Index));
		if (UsedViews.Contains(Value))
		{
			continue;
		}

		TSharedPtr<IPropertyHandle> Handle = ViewHandle;
		MenuBuilder.AddMenuEntry(
			Enum->GetDisplayNameTextByIndex(Index),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([Handle, Value]()
		{
			Handle->SetValue(Value);
		})));
	}

	return MenuBuilder.MakeWidget();
}

FText FSentryCrashReporterViewCustomization::GetCurrentViewText() const
{
	uint8 Value = 0;
	if (!ViewHandle.IsValid() || ViewHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return FText::FromString(TEXT("Multiple Values"));
	}

	return StaticEnum<ESentryCrashReporterView>()->GetDisplayNameTextByValue(Value);
}

TSet<uint8> FSentryCrashReporterViewCustomization::GetViewsUsedBySiblings() const
{
	TSet<uint8> UsedViews;

	const TSharedPtr<IPropertyHandle> ParentHandle = ViewHandle.IsValid() ? ViewHandle->GetParentHandle() : nullptr;
	const TSharedPtr<IPropertyHandleArray> ArrayHandle = ParentHandle.IsValid() ? ParentHandle->AsArray() : nullptr;
	if (!ArrayHandle.IsValid())
	{
		return UsedViews;
	}

	uint32 NumElements = 0;
	ArrayHandle->GetNumElements(NumElements);

	const int32 OwnIndex = ViewHandle->GetIndexInArray();
	for (uint32 Index = 0; Index < NumElements; ++Index)
	{
		if (static_cast<int32>(Index) == OwnIndex)
		{
			continue;
		}

		uint8 Value = 0;
		if (ArrayHandle->GetElement(Index)->GetValue(Value) == FPropertyAccess::Success)
		{
			UsedViews.Add(Value);
		}
	}

	return UsedViews;
}
