// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryBreadcrumb.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryBreadcrumb::FGenericPlatformSentryBreadcrumb()
{
	Breadcrumb = sentry_value_new_breadcrumb("", "");
}

FGenericPlatformSentryBreadcrumb::FGenericPlatformSentryBreadcrumb(sentry_value_t breadcrumb)
{
	Breadcrumb = breadcrumb;
}

sentry_value_t FGenericPlatformSentryBreadcrumb::GetNativeObject()
{
	return Breadcrumb;
}

void FGenericPlatformSentryBreadcrumb::SetMessage(const FString& message)
{
	sentry_value_set_by_key(Breadcrumb, "message", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
}

FString FGenericPlatformSentryBreadcrumb::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(Breadcrumb, "message");
	return FString(sentry_value_as_string(message));
}

void FGenericPlatformSentryBreadcrumb::SetType(const FString& type)
{
	sentry_value_set_by_key(Breadcrumb, "type", sentry_value_new_string(TCHAR_TO_ANSI(*type)));
}

FString FGenericPlatformSentryBreadcrumb::GetType() const
{
	sentry_value_t type = sentry_value_get_by_key(Breadcrumb, "type");
	return FString(sentry_value_as_string(type));
}

void FGenericPlatformSentryBreadcrumb::SetCategory(const FString& category)
{
	sentry_value_set_by_key(Breadcrumb, "category", sentry_value_new_string(TCHAR_TO_ANSI(*category)));
}

FString FGenericPlatformSentryBreadcrumb::GetCategory() const
{
	sentry_value_t category = sentry_value_get_by_key(Breadcrumb, "category");
	return FString(sentry_value_as_string(category));
}

void FGenericPlatformSentryBreadcrumb::SetData(const ::TMap<FString, FSentryVariant>& data)
{
	sentry_value_set_by_key(Breadcrumb, "data", FGenericPlatformSentryConverters::VariantMapToNative(data));
}

TMap<FString, FSentryVariant> FGenericPlatformSentryBreadcrumb::GetData() const
{
	sentry_value_t data = sentry_value_get_by_key(Breadcrumb, "data");
	return FGenericPlatformSentryConverters::VariantMapToUnreal(data);
}

void FGenericPlatformSentryBreadcrumb::SetLevel(ESentryLevel level)
{
	FString levelStr = FGenericPlatformSentryConverters::SentryLevelToString(level);
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(Breadcrumb, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel FGenericPlatformSentryBreadcrumb::GetLevel() const
{
	sentry_value_t level = sentry_value_get_by_key(Breadcrumb, "level");
	return FGenericPlatformSentryConverters::SentryLevelToUnreal(level);
}

#endif
