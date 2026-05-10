// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryMetric.h"

#if !USE_SENTRY_NATIVE

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryMetric::FAppleSentryMetric()
{
	MetricApple = [[SENTRY_APPLE_CLASS(SentryObjCMetric) alloc] initWithTimestamp:[NSDate date]
																			 name:@""
																		  traceId:[[SENTRY_APPLE_CLASS(SentryId) alloc] init]
																		   spanId:nil
																			value:[SENTRY_APPLE_CLASS(SentryObjCMetricValue) counterWithValue:0]
																			 unit:nil
																	   attributes:@{}];
}

FAppleSentryMetric::FAppleSentryMetric(SentryObjCMetric* metric)
{
	MetricApple = metric;
}

FAppleSentryMetric::~FAppleSentryMetric()
{
	// Put custom destructor logic here if needed
}

SentryObjCMetric* FAppleSentryMetric::GetNativeObject()
{
	return MetricApple;
}

void FAppleSentryMetric::SetName(const FString& name)
{
	MetricApple.name = name.GetNSString();
}

FString FAppleSentryMetric::GetName() const
{
	return FString(MetricApple.name);
}

void FAppleSentryMetric::SetType(const FString& type)
{
	float currentValue = GetValue();

	if (type == TEXT("counter"))
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) counterWithValue:(unsigned long long)currentValue];
	}
	else if (type == TEXT("gauge"))
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) gaugeWithValue:(double)currentValue];
	}
	else if (type == TEXT("distribution"))
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) distributionWithValue:(double)currentValue];
	}
}

FString FAppleSentryMetric::GetType() const
{
	switch (MetricApple.value.type)
	{
	case SentryObjCMetricValueTypeCounter:
		return TEXT("counter");
	case SentryObjCMetricValueTypeGauge:
		return TEXT("gauge");
	case SentryObjCMetricValueTypeDistribution:
		return TEXT("distribution");
	default:
		return FString();
	}
}

void FAppleSentryMetric::SetValue(float value)
{
	switch (MetricApple.value.type)
	{
	case SentryObjCMetricValueTypeCounter:
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) counterWithValue:(unsigned long long)value];
		break;
	case SentryObjCMetricValueTypeGauge:
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) gaugeWithValue:(double)value];
		break;
	case SentryObjCMetricValueTypeDistribution:
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) distributionWithValue:(double)value];
		break;
	}
}

float FAppleSentryMetric::GetValue() const
{
	switch (MetricApple.value.type)
	{
	case SentryObjCMetricValueTypeCounter:
		return static_cast<float>(MetricApple.value.counterValue);
	case SentryObjCMetricValueTypeGauge:
		return static_cast<float>(MetricApple.value.gaugeValue);
	case SentryObjCMetricValueTypeDistribution:
		return static_cast<float>(MetricApple.value.distributionValue);
	default:
		return 0.0f;
	}
}

void FAppleSentryMetric::SetUnit(const FString& unit)
{
	MetricApple.unit = unit.GetNSString();
}

FString FAppleSentryMetric::GetUnit() const
{
	return MetricApple.unit ? FString(MetricApple.unit) : FString();
}

void FAppleSentryMetric::SetAttribute(const FString& key, const FSentryVariant& value)
{
	SentryObjCAttributeContent* content = FAppleSentryConverters::VariantToAttributeContentNative(value);

	if (!content)
	{
		return;
	}

	NSMutableDictionary* mutableAttributes = [NSMutableDictionary dictionaryWithDictionary:MetricApple.attributes];
	[mutableAttributes setObject:content forKey:key.GetNSString()];
	MetricApple.attributes = mutableAttributes;
}

FSentryVariant FAppleSentryMetric::GetAttribute(const FString& key) const
{
	SentryObjCAttributeContent* content = [MetricApple.attributes objectForKey:key.GetNSString()];

	if (!content)
	{
		return FSentryVariant();
	}

	return FAppleSentryConverters::SentryAttributeContentToVariant(content);
}

bool FAppleSentryMetric::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	SentryObjCAttributeContent* content = [MetricApple.attributes objectForKey:key.GetNSString()];

	if (!content)
	{
		return false;
	}

	value = FAppleSentryConverters::SentryAttributeContentToVariant(content);
	return true;
}

void FAppleSentryMetric::RemoveAttribute(const FString& key)
{
	NSMutableDictionary* mutableAttributes = [NSMutableDictionary dictionaryWithDictionary:MetricApple.attributes];
	[mutableAttributes removeObjectForKey:key.GetNSString()];
	MetricApple.attributes = mutableAttributes;
}

void FAppleSentryMetric::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	for (const auto& pair : attributes)
	{
		SetAttribute(pair.Key, pair.Value);
	}
}

#endif // !USE_SENTRY_NATIVE
