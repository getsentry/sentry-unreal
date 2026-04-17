// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USentrySubsystem;

/**
 * Base class for sample integration tests invoked via command-line switches.
 *
 * Inherits TSharedFromThis so that tests with async completion (e.g. FSentryHangTest) can capture
 * AsShared() into their deferred lambdas and stay alive past RunIntegrationTest returning.
 * Synchronous tests don't exercise this — they finish before the dispatch call unwinds —
 * but keeping the mixin on the base makes it a non-issue when future tests go async.
 */
class FSentryBaseIntegrationTest : public TSharedFromThis<FSentryBaseIntegrationTest>
{
public:
	FSentryBaseIntegrationTest(const FString& InName) : TestName(InName) {}
	virtual ~FSentryBaseIntegrationTest() = default;

	virtual void Run() = 0;

	const FString& GetName() const { return TestName; }

	USentrySubsystem* GetSubsystem() const;

	/** Emits TEST_RESULT JSON to stdout, flushes logs, and requests app exit. */
	void CompleteWithResult(bool Success, const FString& Message = TEXT("Test complete"));

	/** Converts 32-char event IDs to UUID format (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX). */
	static FString FormatEventIdWithHyphens(const FString& EventId);

protected:
	FString TestName;
};
