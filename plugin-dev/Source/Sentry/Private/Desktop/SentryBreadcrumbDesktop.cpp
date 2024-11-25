// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumbDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentryBreadcrumbDesktop::SentryBreadcrumbDesktop()
{
	BreadcrumbDesktop = sentry_value_new_breadcrumb("", "");
}

SentryBreadcrumbDesktop::SentryBreadcrumbDesktop(sentry_value_t breadcrumb)
{
	BreadcrumbDesktop = breadcrumb;
}

SentryBreadcrumbDesktop::~SentryBreadcrumbDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_value_t SentryBreadcrumbDesktop::GetNativeObject()
{
	return BreadcrumbDesktop;
}

void SentryBreadcrumbDesktop::SetMessage(const FString& message)
{
	sentry_value_set_by_key(BreadcrumbDesktop, "message", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
}

FString SentryBreadcrumbDesktop::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(BreadcrumbDesktop, "message");
	return FString(sentry_value_as_string(message));
}

void SentryBreadcrumbDesktop::SetType(const FString& type)
{
	sentry_value_set_by_key(BreadcrumbDesktop, "type", sentry_value_new_string(TCHAR_TO_ANSI(*type)));
}

FString SentryBreadcrumbDesktop::GetType() const
{
	sentry_value_t type = sentry_value_get_by_key(BreadcrumbDesktop, "type");
	return FString(sentry_value_as_string(type));
}

void SentryBreadcrumbDesktop::SetCategory(const FString& category)
{
	sentry_value_set_by_key(BreadcrumbDesktop, "category", sentry_value_new_string(TCHAR_TO_ANSI(*category)));
}

FString SentryBreadcrumbDesktop::GetCategory() const
{
	sentry_value_t category = sentry_value_get_by_key(BreadcrumbDesktop, "category");
	return FString(sentry_value_as_string(category));
}

void SentryBreadcrumbDesktop::SetData(const TMap<FString, FString>& data)
{
	sentry_value_set_by_key(BreadcrumbDesktop, "data", SentryConvertorsDesktop::StringMapToNative(data));
}

TMap<FString, FString> SentryBreadcrumbDesktop::GetData() const
{
	sentry_value_t data = sentry_value_get_by_key(BreadcrumbDesktop, "data");
	return SentryConvertorsDesktop::StringMapToUnreal(data);
}

void SentryBreadcrumbDesktop::SetLevel(ESentryLevel level)
{
	FString levelStr = SentryConvertorsDesktop::SentryLevelToString(level);
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(BreadcrumbDesktop, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel SentryBreadcrumbDesktop::GetLevel() const
{
	sentry_value_t level = sentry_value_get_by_key(BreadcrumbDesktop, "level");
	return SentryConvertorsDesktop::SentryLevelToUnreal(level);
}

#endif