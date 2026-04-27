// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryPlaygroundBlueprintUtils.generated.h"

UCLASS()
class USentryPlaygroundBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
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
