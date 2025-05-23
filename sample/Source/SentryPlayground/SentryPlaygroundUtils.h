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
	FastFail,
	Assert,
	Ensure
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
};
