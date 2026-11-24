// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

class IDetailChildrenBuilder;

class FSentryCrashReporterImageryCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	FString GetCrashReporterLogoPath(const FString& FileName) const;
};
