// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SentrySubsystem.generated.h"

class USentrySettings;

UCLASS()
class SENTRYSDK_API USentrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Initializes Sentry SDK with given settings. Value specified in ProjectSettings > Plugins > SentrySDK are used by default.
	 *
	 * @param sentrySettings Settings object to support runtime configuration changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize(USentrySettings* sentrySettings);
};
