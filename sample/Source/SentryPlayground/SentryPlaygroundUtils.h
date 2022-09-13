#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryPlaygroundUtils.generated.h"

UCLASS()
class USentryPlaygroundUtils : public UBlueprintFunctionLibrary
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
