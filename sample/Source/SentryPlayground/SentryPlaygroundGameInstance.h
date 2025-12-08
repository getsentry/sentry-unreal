// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SentryPlaygroundUtils.h"
#include "SentrySubsystem.h"
#include "SentryPlaygroundGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SENTRYPLAYGROUND_API USentryPlaygroundGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

private:
	void RunIntegrationTest(const FString& CommandLine);
	void RunCrashTest();
	void RunMessageTest();
	void RunInitOnly();

	void ConfigureTestContext();

	void CompleteTestWithResult(const FString& TestName, bool Result, const FString& Message);

	/** Converts event ID to UUID format with hyphens (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX) */
	static FString FormatEventIdWithHyphens(const FString& EventId);
};
