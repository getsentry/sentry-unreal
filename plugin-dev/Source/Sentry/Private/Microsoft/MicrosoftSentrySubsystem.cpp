// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "Misc/Paths.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "GenericPlatform/GenericPlatformSentryAttachment.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "GenericPlatform/GenericPlatformMisc.h"
#endif

void FMicrosoftSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryTraceSampler* TraceSampler)
{
	FGenericPlatformSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);

#if !UE_VERSION_OLDER_THAN(5, 2, 0)
	if (IsEnabled())
	{
		FPlatformMisc::SetCrashHandlingType(Settings->EnableAutoCrashCapturing ? ECrashHandlingType::Disabled : ECrashHandlingType::Default);
	}

	if (FPlatformMisc::GetCrashHandlingType() == ECrashHandlingType::Default)
	{
		InitCrashReporter(Settings->Release, Settings->Environment);
	}
#else
	InitCrashReporter(Settings->Release, Settings->Environment);
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

#endif // USE_SENTRY_NATIVE
