// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryBreadcrumbApple::SentryBreadcrumbApple()
{
	BreadcrumbApple = [[SENTRY_APPLE_CLASS(SentryBreadcrumb) alloc] init];
}

SentryBreadcrumbApple::SentryBreadcrumbApple(SentryBreadcrumb* breadcrumb)
{
	BreadcrumbApple = breadcrumb;
}

SentryBreadcrumbApple::~SentryBreadcrumbApple()
{
	// Put custom destructor logic here if needed
}

SentryBreadcrumb* SentryBreadcrumbApple::GetNativeObject()
{
	return BreadcrumbApple;
}

void SentryBreadcrumbApple::SetMessage(const FString& message)
{
	BreadcrumbApple.message = message.GetNSString();
}

FString SentryBreadcrumbApple::GetMessage() const
{
	return FString(BreadcrumbApple.message);
}

void SentryBreadcrumbApple::SetType(const FString& type)
{
	BreadcrumbApple.type = type.GetNSString();
}

FString SentryBreadcrumbApple::GetType() const
{
	return FString(BreadcrumbApple.type);
}

void SentryBreadcrumbApple::SetCategory(const FString& category)
{
	BreadcrumbApple.category = category.GetNSString();
}

FString SentryBreadcrumbApple::GetCategory() const
{
	return FString(BreadcrumbApple.category);
}

void SentryBreadcrumbApple::SetData(const TMap<FString, FString>& data)
{
	BreadcrumbApple.data = SentryConvertorsApple::StringMapToNative(data);
}

TMap<FString, FString> SentryBreadcrumbApple::GetData() const
{
	return SentryConvertorsApple::StringMapToUnreal(BreadcrumbApple.data);
}

void SentryBreadcrumbApple::SetLevel(ESentryLevel level)
{
	BreadcrumbApple.level = SentryConvertorsApple::SentryLevelToNative(level);
}

ESentryLevel SentryBreadcrumbApple::GetLevel() const
{
	return SentryConvertorsApple::SentryLevelToUnreal(BreadcrumbApple.level);
}
