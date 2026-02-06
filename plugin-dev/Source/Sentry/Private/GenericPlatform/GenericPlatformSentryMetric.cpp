// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryMetric.h"

#if USE_SENTRY_NATIVE

#include "Infrastructure/GenericPlatformSentryConverters.h"

FGenericPlatformSentryMetric::FGenericPlatformSentryMetric()
{
	Metric = sentry_value_new_object();
}

FGenericPlatformSentryMetric::FGenericPlatformSentryMetric(sentry_value_t metric)
{
	Metric = metric;
}

sentry_value_t FGenericPlatformSentryMetric::GetNativeObject()
{
	return Metric;
}

void FGenericPlatformSentryMetric::SetName(const FString& name)
{
	sentry_value_set_by_key(Metric, "name", sentry_value_new_string(TCHAR_TO_UTF8(*name)));
}

FString FGenericPlatformSentryMetric::GetName() const
{
	sentry_value_t name = sentry_value_get_by_key(Metric, "name");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(name)));
}

void FGenericPlatformSentryMetric::SetType(const FString& type)
{
	sentry_value_set_by_key(Metric, "type", sentry_value_new_string(TCHAR_TO_UTF8(*type)));
}

FString FGenericPlatformSentryMetric::GetType() const
{
	sentry_value_t type = sentry_value_get_by_key(Metric, "type");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(type)));
}

void FGenericPlatformSentryMetric::SetValue(double value)
{
	sentry_value_set_by_key(Metric, "value", sentry_value_new_double(value));
}

double FGenericPlatformSentryMetric::GetValue() const
{
	sentry_value_t value = sentry_value_get_by_key(Metric, "value");
	return sentry_value_as_double(value);
}

void FGenericPlatformSentryMetric::SetUnit(const FString& unit)
{
	sentry_value_set_by_key(Metric, "unit", sentry_value_new_string(TCHAR_TO_UTF8(*unit)));
}

FString FGenericPlatformSentryMetric::GetUnit() const
{
	sentry_value_t unit = sentry_value_get_by_key(Metric, "unit");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(unit)));
}

void FGenericPlatformSentryMetric::SetAttribute(const FString& key, const FSentryVariant& value)
{
	sentry_value_t attributes = sentry_value_get_by_key(Metric, "attributes");
	if (sentry_value_is_null(attributes))
	{
		attributes = sentry_value_new_object();
		sentry_value_set_by_key(Metric, "attributes", attributes);
	}

	sentry_value_t attribute = FGenericPlatformSentryConverters::VariantToAttributeNative(value);
	sentry_value_set_by_key(attributes, TCHAR_TO_UTF8(*key), attribute);
}

FSentryVariant FGenericPlatformSentryMetric::GetAttribute(const FString& key) const
{
	sentry_value_t attributes = sentry_value_get_by_key(Metric, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return FSentryVariant();
	}

	sentry_value_t attribute = sentry_value_get_by_key(attributes, TCHAR_TO_UTF8(*key));
	if (sentry_value_is_null(attribute))
	{
		return FSentryVariant();
	}

	sentry_value_t attributeValue = sentry_value_get_by_key(attribute, "value");
	return FGenericPlatformSentryConverters::VariantToUnreal(attributeValue);
}

bool FGenericPlatformSentryMetric::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	sentry_value_t attributes = sentry_value_get_by_key(Metric, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return false;
	}

	sentry_value_t attribute = sentry_value_get_by_key(attributes, TCHAR_TO_UTF8(*key));
	if (sentry_value_is_null(attribute))
	{
		return false;
	}

	sentry_value_t attributeValue = sentry_value_get_by_key(attribute, "value");
	if (sentry_value_is_null(attributeValue))
	{
		return false;
	}

	value = FGenericPlatformSentryConverters::VariantToUnreal(attributeValue);

	return true;
}

void FGenericPlatformSentryMetric::RemoveAttribute(const FString& key)
{
	sentry_value_t attributes = sentry_value_get_by_key(Metric, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return;
	}

	sentry_value_remove_by_key(attributes, TCHAR_TO_UTF8(*key));
}

void FGenericPlatformSentryMetric::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	for (const auto& pair : attributes)
	{
		SetAttribute(pair.Key, pair.Value);
	}
}

#endif // USE_SENTRY_NATIVE
