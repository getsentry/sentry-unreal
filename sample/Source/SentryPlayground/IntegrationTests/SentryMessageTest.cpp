// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryMessageTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentryBreadcrumb.h"
#include "SentryScope.h"
#include "SentrySubsystem.h"
#include "SentryVariant.h"

#include "HAL/PlatformProcess.h"
#include "Misc/EngineVersionComparison.h"

void FSentryMessageTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	// Override release and environment to verify SetRelease/SetEnvironment work post-init
	Subsystem->SetRelease(TEXT("test-release@1.0.0"));
	Subsystem->SetEnvironment(TEXT("test-environment"));

	// Start a new session to apply the new release and environment values
	Subsystem->StartSession();

	FString EventId = Subsystem->CaptureMessageWithScope(TEXT("Integration test message"), FConfigureScopeNativeDelegate::CreateLambda([](USentryScope* Scope)
		{
			// Local scope tag
			Scope->SetTag(TEXT("scope.locality"), TEXT("local"));

			// Local scope extras (one persists, one for beforeSend to remove)
			Scope->SetExtra(TEXT("local_extra"), FSentryVariant(TEXT("local_extra_value")));
			Scope->SetExtra(TEXT("extra_to_be_removed"), FSentryVariant(TEXT("original_value")));

			// Local scope context
			TMap<FString, FSentryVariant> LocalContext;
			LocalContext.Add(TEXT("local_key"), FSentryVariant(TEXT("local_value")));
			Scope->SetContext(TEXT("local_context"), LocalContext);

			// Local scope breadcrumb
			USentryBreadcrumb* Breadcrumb = NewObject<USentryBreadcrumb>();
			Breadcrumb->Initialize();
			Breadcrumb->SetMessage(TEXT("Local scope breadcrumb"));
			Breadcrumb->SetCategory(TEXT("test"));
			Breadcrumb->SetType(TEXT("info"));
			Scope->AddBreadcrumb(Breadcrumb);
		}),
		ESentryLevel::Info);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#endif

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(!EventId.IsEmpty());
}
