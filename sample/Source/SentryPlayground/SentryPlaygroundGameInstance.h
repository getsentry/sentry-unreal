// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SentryPlaygroundGameInstance.generated.h"

class FSentryBaseIntegrationTest;
class USentrySubsystem;

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
	/** Maps a command-line switch to a factory that builds the corresponding integration test. */
	struct FIntegrationTestEntry
	{
		const TCHAR* Param;
		TFunction<TSharedPtr<FSentryBaseIntegrationTest>()> MakeTest;
	};

	/**
	 * Inspects the command line for a test-mode switch (e.g. `-crash-capture`).
	 * Returns the matching test instance, or nullptr if no switch is present
	 * in which case the game should launch normally into the sample UI.
	 */
	TSharedPtr<FSentryBaseIntegrationTest> CheckForPendingIntegrationTest(const FString& CommandLine) const;

	void RunIntegrationTest(TSharedRef<FSentryBaseIntegrationTest> Test);

	void ConfigureTestContext(USentrySubsystem* Subsystem);
};
