// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryPlaygroundUtils.generated.h"

UENUM(BlueprintType)
enum class ESentryAppTerminationType : uint8
{
	NullPointer,
	ArrayOutOfBounds,
	BadFunctionPtr,
	InvalidMemoryAccess,
	StackOverflow,
	OutOfMemory,
	FastFail,
	RenderThreadCrash,
	GpuDebugCrash,
	Assert,
	Ensure,
	Fatal
};

UCLASS()
class USentryPlaygroundUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Crashes the application. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void Terminate(ESentryAppTerminationType Type);

	/** Converts string to byte array. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static TArray<uint8> StringToBytesArray(const FString& InString);

	/** Converts byte array to string. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static FString ByteArrayToString(const TArray<uint8>& Array);

	/**
	 * Saves string to a file.
	 *
	 * @param InString Content of a file.
	 * @param Filename Name of a file to be saved.
	 *
	 * @return Absolute path to a file with specified name and content.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static FString SaveStringToFile(const FString& InString, const FString& Filename);

	/** Forces garbage collection and captures Sentry event to test beforeSend hook limitation. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void CaptureEventDuringGC();

	/**
	 * Logs diagnostic information about Crashpad exception handling state.
	 * Useful for debugging crash capture issues on Wine/Proton.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Diagnostics")
	static void LogCrashpadDiagnostics();

	/**
	 * Checks if an exception filter is currently installed.
	 * Returns true if Crashpad or another crash handler has registered an exception filter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Diagnostics")
	static bool IsExceptionFilterInstalled();
};
