// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryMeasurementUnit.h"

FSentryMeasurementUnit::FSentryMeasurementUnit(ESentryMeasurementUnit InUnit)
	: Unit(InUnit)
{
}

FSentryMeasurementUnit::FSentryMeasurementUnit(const FString& InCustomUnit)
	: Unit(ESentryMeasurementUnit::Custom)
	, CustomUnit(InCustomUnit)
{
}

FString FSentryMeasurementUnit::ToString() const
{
	if (Unit == ESentryMeasurementUnit::None)
	{
		return TEXT("");
	}

	if (Unit == ESentryMeasurementUnit::Custom)
	{
		return CustomUnit;
	}

	return StaticEnum<ESentryMeasurementUnit>()->GetNameStringByValue(static_cast<int64>(Unit)).ToLower();
}
