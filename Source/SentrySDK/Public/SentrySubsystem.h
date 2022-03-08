// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SentryDataTypes.h"

#include "SentrySubsystem.generated.h"

class USentrySettings;

UCLASS()
class SENTRYSDK_API USentrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Initializes Sentry SDK with values specified in ProjectSettings > Plugins > SentrySDK. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();
	
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureMessage(const FString& Message, ESentryLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureError();
};
