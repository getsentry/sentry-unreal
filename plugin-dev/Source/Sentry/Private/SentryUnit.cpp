// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryUnit.h"

FSentryUnit::FSentryUnit()
	: Unit(ESentryUnit::None)
{
}

FSentryUnit::FSentryUnit(ESentryUnit InUnit)
	: Unit(InUnit)
{
}

FSentryUnit::FSentryUnit(const FString& InCustomUnit)
	: Unit(ESentryUnit::None), CustomUnit(InCustomUnit)
{
}

FString FSentryUnit::ToString() const
{
	if (!CustomUnit.IsEmpty())
	{
		return CustomUnit;
	}

	if (Unit == ESentryUnit::None)
	{
		return TEXT("");
	}

	return StaticEnum<ESentryUnit>()->GetNameStringByValue(static_cast<int64>(Unit)).ToLower();
}

FSentryUnit USentryUnitHelper::MakeSentryUnit(ESentryUnit Unit)
{
	return FSentryUnit(Unit);
}

FSentryUnit USentryUnitHelper::MakeSentryCustomUnit(const FString& Unit)
{
	return FSentryUnit(Unit);
}

FString USentryUnitHelper::ToString(const FSentryUnit& Unit)
{
	return Unit.ToString();
}

