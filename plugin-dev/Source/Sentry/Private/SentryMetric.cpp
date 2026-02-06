// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryMetric.h"

#include "Interface/SentryMetricInterface.h"

#include "HAL/PlatformSentryMetric.h"

void USentryMetric::Initialize()
{
	NativeImpl = MakeShareable(new FPlatformSentryMetric);
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

void USentryMetric::SetType(const FString& InType)
{
	if (NativeImpl)
		NativeImpl->SetType(InType);
}

FString USentryMetric::GetType() const
{
	if (NativeImpl)
		return NativeImpl->GetType();

	return FString();
}

void USentryMetric::SetValue(float InValue)
{
	if (NativeImpl)
		NativeImpl->SetValue((double)InValue);
}

float USentryMetric::GetValue() const
{
	if (NativeImpl)
		return (float)NativeImpl->GetValue();

	return 0.0f;
}

void USentryMetric::SetUnit(const FString& InUnit)
{
	if (NativeImpl)
		NativeImpl->SetUnit(InUnit);
}

FString USentryMetric::GetUnit() const
{
	if (NativeImpl)
		return NativeImpl->GetUnit();

	return FString();
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
