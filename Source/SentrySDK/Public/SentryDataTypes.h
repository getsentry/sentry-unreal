// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

UENUM(BlueprintType)
enum class ESentryLevel : uint8
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};
