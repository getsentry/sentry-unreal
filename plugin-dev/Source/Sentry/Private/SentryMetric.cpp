// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryMetric.h"

#include "Interface/SentryMetricInterface.h"

#include "HAL/PlatformSentryMetric.h"

void USentryMetric::Initialize()
{
	NativeImpl = CreateSharedSentryMetric();
}

void USentryMetric::SetName(const FString& InName)
{
	if (NativeImpl)
		NativeImpl->SetName(InName);
}

FString USentryMetric::GetName() const
{
	if (NativeImpl)
		return NativeImpl->GetName();

	return FString();
}

void USentryMetric::SetType(ESentryMetricType InType)
{
	if (InType == ESentryMetricType::Unknown)
		return;

	if (NativeImpl)
		NativeImpl->SetType(MetricTypeToString(InType));
}

ESentryMetricType USentryMetric::GetType() const
{
	if (NativeImpl)
		return StringToMetricType(NativeImpl->GetType());

	return ESentryMetricType::Unknown;
}

void USentryMetric::SetValue(float InValue)
{
	if (NativeImpl)
		NativeImpl->SetValue(InValue);
}

float USentryMetric::GetValue() const
{
	if (NativeImpl)
		return NativeImpl->GetValue();

	return 0.0f;
}

void USentryMetric::SetUnit(const FSentryUnit& InUnit)
{
	if (NativeImpl)
		NativeImpl->SetUnit(InUnit.ToString());
}

FSentryUnit USentryMetric::GetUnit() const
{
	if (NativeImpl)
		return FSentryUnit(NativeImpl->GetUnit());

	return FSentryUnit();
}

void USentryMetric::SetAttribute(const FString& Key, const FSentryVariant& Value)
{
	if (NativeImpl)
		NativeImpl->SetAttribute(Key, Value);
}

FSentryVariant USentryMetric::GetAttribute(const FString& Key) const
{
	if (NativeImpl)
		return NativeImpl->GetAttribute(Key);

	return FSentryVariant();
}

bool USentryMetric::TryGetAttribute(const FString& Key, FSentryVariant& Value) const
{
	if (NativeImpl)
		return NativeImpl->TryGetAttribute(Key, Value);

	return false;
}

void USentryMetric::RemoveAttribute(const FString& Key)
{
	if (NativeImpl)
		NativeImpl->RemoveAttribute(Key);
}

void USentryMetric::AddAttributes(const TMap<FString, FSentryVariant>& Attributes)
{
	if (NativeImpl)
		NativeImpl->AddAttributes(Attributes);
}

FString USentryMetric::MetricTypeToString(ESentryMetricType Type)
{
	switch (Type)
	{
	case ESentryMetricType::Counter:
		return TEXT("counter");
	case ESentryMetricType::Gauge:
		return TEXT("gauge");
	case ESentryMetricType::Distribution:
		return TEXT("distribution");
	default:
		return TEXT("");
	}
}

ESentryMetricType USentryMetric::StringToMetricType(const FString& Type)
{
	if (Type == TEXT("counter"))
		return ESentryMetricType::Counter;
	if (Type == TEXT("gauge"))
		return ESentryMetricType::Gauge;
	if (Type == TEXT("distribution"))
		return ESentryMetricType::Distribution;

	return ESentryMetricType::Unknown;
}
