// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryPlaygroundCrashUtils.generated.h"

UENUM(BlueprintType)
enum class ESentryAppTerminationType : uint8
{
	NullPointer,
	ArrayOutOfBounds,
	BadFunctionPtr,
	InvalidMemoryAccess,
	StackOverflow,
	OutOfMemory,
	MemoryCorruption,
	FastFail,
	RenderThreadCrash,
	GpuDebugCrash,
	Assert,
	AssertReentrant,
	Ensure,
	Fatal,
	Hang
};

UCLASS()
class USentryPlaygroundCrashUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Crashes the application. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void Terminate(ESentryAppTerminationType Type);

	/** Forces garbage collection and captures Sentry event to test beforeSend hook limitation. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void CaptureEventDuringGC();
};
