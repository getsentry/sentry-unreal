// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryMetric.h"

#if !USE_SENTRY_NATIVE

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryMetric::FAppleSentryMetric()
{
	MetricApple = [[SENTRY_APPLE_CLASS(SentryObjCMetric) alloc] init];
	MetricApple.timestamp = [NSDate date];
	MetricApple.name = @"";
	MetricApple.traceId = [[SENTRY_APPLE_CLASS(SentryObjCId) alloc] init];
	MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) counter:0];
	MetricApple.attributes = @{};
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
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) counter:(unsigned long long)currentValue];
	}
	else if (type == TEXT("gauge"))
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) gauge:(double)currentValue];
	}
	else if (type == TEXT("distribution"))
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) distribution:(double)currentValue];
	}
}

FString FAppleSentryMetric::GetType() const
{
	if (MetricApple.value.isCounter)
	{
		return TEXT("counter");
	}
	else if (MetricApple.value.isGauge)
	{
		return TEXT("gauge");
	}
	else if (MetricApple.value.isDistribution)
	{
		return TEXT("distribution");
	}

	return FString();
}

void FAppleSentryMetric::SetValue(float value)
{
	if (MetricApple.value.isCounter)
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) counter:(unsigned long long)value];
	}
	else if (MetricApple.value.isGauge)
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) gauge:(double)value];
	}
	else if (MetricApple.value.isDistribution)
	{
		MetricApple.value = [SENTRY_APPLE_CLASS(SentryObjCMetricValue) distribution:(double)value];
	}
}

float FAppleSentryMetric::GetValue() const
{
	if (MetricApple.value.isCounter)
	{
		return static_cast<float>(MetricApple.value.counterValue);
	}
	else if (MetricApple.value.isGauge)
	{
		return static_cast<float>(MetricApple.value.gaugeValue);
	}
	else if (MetricApple.value.isDistribution)
	{
		return static_cast<float>(MetricApple.value.distributionValue);
	}

	return 0.0f;
}

void FAppleSentryMetric::SetUnit(const FString& unit)
{
	if (unit.IsEmpty())
	{
		MetricApple.unit = nil;
	}
	else
	{
		MetricApple.unit = [[SENTRY_APPLE_CLASS(SentryObjCUnit) alloc] initWithRawValue:unit.GetNSString()];
	}
}

FString FAppleSentryMetric::GetUnit() const
{
	return MetricApple.unit ? FString(MetricApple.unit.rawValue) : FString();
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
