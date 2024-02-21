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
	Assert
};

UCLASS()
class USentryPlaygroundUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Crashes the application. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void Terminate(ESentryAppTerminationType Type);
};
