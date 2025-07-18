// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryBreadcrumb.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryBreadcrumb::FAppleSentryBreadcrumb()
{
	BreadcrumbApple = [[SENTRY_APPLE_CLASS(SentryBreadcrumb) alloc] init];
}

FAppleSentryBreadcrumb::FAppleSentryBreadcrumb(SentryBreadcrumb* breadcrumb)
{
	BreadcrumbApple = breadcrumb;
}

FAppleSentryBreadcrumb::~FAppleSentryBreadcrumb()
{
	// Put custom destructor logic here if needed
}

SentryBreadcrumb* FAppleSentryBreadcrumb::GetNativeObject()
{
	return BreadcrumbApple;
}

void FAppleSentryBreadcrumb::SetMessage(const FString& message)
{
	BreadcrumbApple.message = message.GetNSString();
}

FString FAppleSentryBreadcrumb::GetMessage() const
{
	return FString(BreadcrumbApple.message);
}

void FAppleSentryBreadcrumb::SetType(const FString& type)
{
	BreadcrumbApple.type = type.GetNSString();
}

FString FAppleSentryBreadcrumb::GetType() const
{
	return FString(BreadcrumbApple.type);
}

void FAppleSentryBreadcrumb::SetCategory(const FString& category)
{
	BreadcrumbApple.category = category.GetNSString();
}

FString FAppleSentryBreadcrumb::GetCategory() const
{
	return FString(BreadcrumbApple.category);
}

void FAppleSentryBreadcrumb::SetData(const TMap<FString, FSentryVariant>& data)
{
	BreadcrumbApple.data = FAppleSentryConverters::VariantMapToNative(data);
}

TMap<FString, FSentryVariant> FAppleSentryBreadcrumb::GetData() const
{
	return FAppleSentryConverters::VariantMapToUnreal(BreadcrumbApple.data);
}

void FAppleSentryBreadcrumb::SetLevel(ESentryLevel level)
{
	BreadcrumbApple.level = FAppleSentryConverters::SentryLevelToNative(level);
}

ESentryLevel FAppleSentryBreadcrumb::GetLevel() const
{
	return FAppleSentryConverters::SentryLevelToUnreal(BreadcrumbApple.level);
}
