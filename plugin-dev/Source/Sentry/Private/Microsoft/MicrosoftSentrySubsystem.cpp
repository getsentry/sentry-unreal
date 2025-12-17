// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "Microsoft/MicrosoftSentryCrashLogger.h"

#include "GenericPlatform/GenericPlatformSentryAttachment.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"

#include "Microsoft/AllowMicrosoftPlatformTypes.h"

#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "GenericPlatform/GenericPlatformMisc.h"
#endif

void FMicrosoftSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryBeforeLogHandler* BeforeLogHandler, USentryTraceSampler* TraceSampler)
{
	// Initialize crash logger if enabled
	if (Settings->EnableOnCrashLogging)
	{
		CrashLogger = MakeUnique<FMicrosoftSentryCrashLogger>();
		if (CrashLogger->IsThreadRunning())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Crash logging enabled - stack traces will be written to game log during crashes"));
		}
		else
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Crash logging requested but thread failed to initialize"));
			CrashLogger.Reset();
		}
	}

	FGenericPlatformSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, BeforeLogHandler, TraceSampler);

#if !UE_VERSION_OLDER_THAN(5, 2, 0)
	if (IsEnabled())
	{
		FPlatformMisc::SetCrashHandlingType(Settings->EnableAutoCrashCapturing ? ECrashHandlingType::Disabled : ECrashHandlingType::Default);
	}

	if (FPlatformMisc::GetCrashHandlingType() == ECrashHandlingType::Default)
	{
		InitCrashReporter(Settings->GetEffectiveRelease(), Settings->GetEffectiveEnvironment());
	}
#else
	InitCrashReporter(Settings->GetEffectiveRelease(), Settings->GetEffectiveEnvironment());
#endif
}

void FMicrosoftSentrySubsystem::ConfigureDatabasePath(sentry_options_t* Options)
{
	sentry_options_set_database_pathw(Options, *GetDatabasePath());
}

void FMicrosoftSentrySubsystem::ConfigureLogFileAttachment(sentry_options_t* Options)
{
	const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();
	sentry_options_add_attachmentw(Options, *FPaths::ConvertRelativePathToFull(LogFilePath));
}

void FMicrosoftSentrySubsystem::AddFileAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	sentry_attachment_t* nativeAttachment =
		sentry_attach_filew(*platformAttachment->GetPath());

	if (!platformAttachment->GetFilename().IsEmpty())
		sentry_attachment_set_filenamew(nativeAttachment, *platformAttachment->GetFilename());

	if (!platformAttachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*platformAttachment->GetContentType()));

	platformAttachment->SetNativeObject(nativeAttachment);

	attachments.Add(platformAttachment);
}

void FMicrosoftSentrySubsystem::AddByteAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	const TArray<uint8>& byteBuf = platformAttachment->GetDataByRef();

	sentry_attachment_t* nativeAttachment =
		sentry_attach_bytesw(reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), *platformAttachment->GetFilename());

	if (!platformAttachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*platformAttachment->GetContentType()));

	platformAttachment->SetNativeObject(nativeAttachment);

	attachments.Add(platformAttachment);
}

sentry_value_t FMicrosoftSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	// If crash logging is enabled, log the crash callstack to game log
	if (CrashLogger)
	{
		// Get a pseudo-handle to the current thread (the crashed thread)
		// We need a real handle for cross-thread stack walking
		HANDLE CurrentThreadPseudoHandle = GetCurrentThread();
		HANDLE CrashedThreadHandle = nullptr;

		// Duplicate the pseudo-handle to get a real handle
		if (DuplicateHandle(
				GetCurrentProcess(),	   // Source process
				CurrentThreadPseudoHandle, // Source handle (pseudo)
				GetCurrentProcess(),	   // Target process
				&CrashedThreadHandle,	   // Target handle (real)
				0,						   // Desired access (ignored when using DUPLICATE_SAME_ACCESS)
				FALSE,					   // Inherit handle
				DUPLICATE_SAME_ACCESS	   // Options
				))
		{
			// Log the crash (with timeout to prevent hanging)
			// This waits for the logging thread to complete stack walking, fill GErrorHist and dump callstack to logs
			// Logging thread is responsible for closing CrashedThreadHandle when done
			CrashLogger->LogCrash(uctx, CrashedThreadHandle, 5000);
		}
	}

	return FGenericPlatformSentrySubsystem::OnCrash(uctx, event, closure);
}

void FMicrosoftSentrySubsystem::Close()
{
	if (CrashLogger)
	{
		CrashLogger.Reset();
	}

	FGenericPlatformSentrySubsystem::Close();
}

#include "Microsoft/HideMicrosoftPlatformTypes.h"

#endif // USE_SENTRY_NATIVE
