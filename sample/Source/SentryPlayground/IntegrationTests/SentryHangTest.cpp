// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryHangTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentrySubsystem.h"

#include "HAL/PlatformProcess.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersionComparison.h"

void FSentryHangTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Flush logs to ensure output is captured before hang
	GLog->Flush();

	Subsystem->SetTag(TEXT("test.hang_id"), EventId);

	// The hang must be triggered only after the game thread has ticked a few frames.
	// Capture AsShared() so this test stays alive past RunIntegrationTest returning.
	TSharedRef<FSentryBaseIntegrationTest> Self = AsShared();
	TSharedRef<int32> FrameCounter = MakeShared<int32>(0);
	TSharedRef<bool> bHangTriggered = MakeShared<bool>(false);

	FCoreDelegates::OnEndFrame.AddLambda([Self, FrameCounter, bHangTriggered]()
	{
		if (*bHangTriggered)
		{
			return;
		}

		constexpr int32 FramesBeforeHang = 3;
		if (++(*FrameCounter) < FramesBeforeHang)
		{
			return;
		}

		*bHangTriggered = true;

		USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType::Hang);

		// On Android the app-hang event is captured by the sentry-native watchdog and written to the
		// outbox; give the SDK a moment to upload it in-session before flushing and exiting.
#if PLATFORM_ANDROID
		FPlatformProcess::Sleep(1.0f);
#endif

		Self->GetSubsystem()->Close();
		Self->CompleteWithResult(true);
	});
}
