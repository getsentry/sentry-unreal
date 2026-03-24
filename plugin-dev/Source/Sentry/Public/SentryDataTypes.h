// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.generated.h"

UENUM(BlueprintType)
enum class ESentryLevel : uint8
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

UENUM(BlueprintType)
enum class ESentryCrashedLastRun : uint8
{
	NotEvaluated,
	NotCrashed,
	Crashed
};

UENUM(BlueprintType)
enum class EUserConsent : uint8
{
	Revoked,
	Given,
	Unknown
};

UENUM(BlueprintType)
enum class ESentryMetricType : uint8
{
	Unknown,
	Counter,
	Gauge,
	Distribution
};

UENUM(BlueprintType)
enum class ESentryMinidumpMode : uint8
{
	// Capture only stack memory. Fastest and smallest, suitable for production.
	StackOnly,
	// Capture stack and heap around crash site. Balanced mode, recommended for most applications.
	Smart,
	// Capture full process memory. Most comprehensive but slowest. Best for development/staging.
	Full,
};

UENUM(BlueprintType)
enum class ESentryCrashReportingMode : uint8
{
	// Write and send minidump for server-side symbolication. No client-side stack unwinding.
	Minidump,
	// Walk stack client-side in crash daemon, send JSON event with stacktraces. No minidump generated.
	NativeStackwalking,
	// Client-side stack unwinding with minidump attached for deep debugging. Best of both worlds.
	NativeStackwalkingWithMinidump,
};
