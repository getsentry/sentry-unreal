// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryBeforeSendHandler.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryAttachment.h"

#include "Utils/SentryFileUtils.h"

#include "Misc/Paths.h"

USentryEvent* USentryBeforeSendHandler::HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->EnableAutoLogAttachment)
	{
#if PLATFORM_ANDROID || PLATFORM_APPLE
		const FString LogFilePath = Event->IsCrash() ? SentryFileUtils::GetGameLogBackupPath() : SentryFileUtils::GetGameLogPath();
#else
		const FString LogFilePath = SentryFileUtils::GetGameLogPath();
#endif

		USentryAttachment* Attachment = NewObject<USentryAttachment>();
		Attachment->InitializeWithPath(LogFilePath, FPaths::GetCleanFilename(LogFilePath), TEXT("text/plain"));

		if(Hint != nullptr)
		{
			Hint->AddAttachment(Attachment);
		}
	}

	return Event;
}
