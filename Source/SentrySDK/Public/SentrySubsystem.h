// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"

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

	/**
	 * Captures the message.
	 *
	 * @param Message The message to send.
	 * @param Level The message level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FGuid CaptureMessage(const FString& Message, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Captures the message with a configurable scope.
	 * This allows modifying a scope without affecting other events.
	 * Changing message level during scope configuration will override Level parameter value.
	 *
	 * @param Message The message to send.
	 * @param OnConfigureScope The callback to configure the scope.
	 * @param Level The message level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "OnCofigureScope"))
	FGuid CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level = ESentryLevel::Info);

	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FGuid CaptureError();
};
