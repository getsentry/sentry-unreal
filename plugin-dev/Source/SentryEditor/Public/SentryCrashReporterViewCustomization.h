// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

class IDetailChildrenBuilder;
class SWidget;

class FSentryCrashReporterViewCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedRef<SWidget> MakeViewMenu() const;
	FText GetCurrentViewText() const;
	TSet<uint8> GetViewsUsedBySiblings() const;

	TSharedPtr<IPropertyHandle> ViewHandle;
};
