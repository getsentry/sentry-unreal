// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBaseIntegrationTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentrySubsystem.h"

#include "Engine/Engine.h"
#include "HAL/PlatformMisc.h"

USentrySubsystem* FSentryBaseIntegrationTest::GetSubsystem() const
{
	return GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
}

void FSentryBaseIntegrationTest::CompleteWithResult(bool Success, const FString& Message)
{
	UE_LOG(LogSentrySample, Display, TEXT("TEST_RESULT: {\"test\":\"%s\",\"success\":%s,\"message\":\"%s\"}\n"),
		*TestName, Success ? TEXT("true") : TEXT("false"), *Message);

	// Flush logs to ensure output is captured before exit
	GLog->Flush();

	// Close app after test is completed
	FPlatformMisc::RequestExitWithStatus(true, 0);
}

FString FSentryBaseIntegrationTest::FormatEventIdWithHyphens(const FString& EventId)
{
	if (EventId.Len() == 32 && !EventId.Contains(TEXT("-")))
	{
		return FString::Printf(TEXT("%s-%s-%s-%s-%s"),
			*EventId.Mid(0, 8),
			*EventId.Mid(8, 4),
			*EventId.Mid(12, 4),
			*EventId.Mid(16, 4),
			*EventId.Mid(20, 12));
	}

	return EventId;
}
