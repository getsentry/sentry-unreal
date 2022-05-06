// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbIOS.h"

#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryBreadcrumbIOS::SentryBreadcrumbIOS()
{
	BreadcrumbIOS = [[SentryBreadcrumb alloc] init];
}

SentryBreadcrumbIOS::SentryBreadcrumbIOS(SentryBreadcrumb* breadcrumb)
{
	BreadcrumbIOS = breadcrumb;
}

SentryBreadcrumbIOS::~SentryBreadcrumbIOS()
{
	// Put custom destructor logic here if needed
}

void SentryBreadcrumbIOS::SetMessage(const FString& message)
{
	BreadcrumbIOS.message = message.GetNSString();
}

FString SentryBreadcrumbIOS::GetMessage() const
{
	return FString(BreadcrumbIOS.message);
}

void SentryBreadcrumbIOS::SetType(const FString& type)
{
	BreadcrumbIOS.type = type.GetNSString();
}

FString SentryBreadcrumbIOS::GetType() const
{
	return FString(BreadcrumbIOS.type);
}

void SentryBreadcrumbIOS::SetCategory(const FString& category)
{
	BreadcrumbIOS.category = category.GetNSString();
}

FString SentryBreadcrumbIOS::GetCategory() const
{
	return FString(BreadcrumbIOS.category);
}

void SentryBreadcrumbIOS::SetData(const TMap<FString, FString>& data)
{
	BreadcrumbIOS.data = SentryConvertorsIOS::StringMapToNative(data);
}

TMap<FString, FString> SentryBreadcrumbIOS::GetData() const
{
	return SentryConvertorsIOS::StringMapToUnreal(BreadcrumbIOS.data);
}

void SentryBreadcrumbIOS::SetLevel(ESentryLevel level)
{
	BreadcrumbIOS.level = SentryConvertorsIOS::SentryLevelToNative(level);
}

ESentryLevel SentryBreadcrumbIOS::GetLevel() const
{
	return SentryConvertorsIOS::SentryLevelToUnreal(BreadcrumbIOS.level);
}
