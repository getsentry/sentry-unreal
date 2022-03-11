// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryLibrary.generated.h"

UCLASS()
class SENTRYSDK_API USentryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Crashes the application. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void Crash();

	/** Generates assert. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void Assert();
};
