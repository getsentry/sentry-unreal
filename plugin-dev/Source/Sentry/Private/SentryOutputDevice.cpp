// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryOutputDevice.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Engine/Engine.h"
#include "Utils/SentryLogUtils.h"

FSentryOutputDevice::FSentryOutputDevice()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	BreadcrumbFlags.Add(ESentryLevel::Fatal, Settings->AutomaticBreadcrumbsForLogs.bOnFatalLog);
	BreadcrumbFlags.Add(ESentryLevel::Error, Settings->AutomaticBreadcrumbsForLogs.bOnErrorLog);
	BreadcrumbFlags.Add(ESentryLevel::Warning, Settings->AutomaticBreadcrumbsForLogs.bOnWarningLog);
	BreadcrumbFlags.Add(ESentryLevel::Info, Settings->AutomaticBreadcrumbsForLogs.bOnInfoLog);
	BreadcrumbFlags.Add(ESentryLevel::Debug, Settings->AutomaticBreadcrumbsForLogs.bOnDebugLog);
}

void FSentryOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	const FString Message = FString(V).TrimStartAndEnd();
	if (Message.IsEmpty() || Message.Contains(TEXT("[Callstack]")))
	{
		return;
	}

	ESentryLevel BreadcrumbLevel = SentryLogUtils::ConvertLogVerbosityToSentryLevel(Verbosity);

	if (!BreadcrumbFlags.Contains(BreadcrumbLevel) || !BreadcrumbFlags[BreadcrumbLevel])
	{
		return;
	}

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!SentrySubsystem || !SentrySubsystem->IsEnabled())
	{
		return;
	}

	SentrySubsystem->AddBreadcrumbWithParams(Message, Category.ToString(), FString(), TMap<FString, FSentryVariant>(), BreadcrumbLevel);
}

bool FSentryOutputDevice::CanBeUsedOnAnyThread() const
{
	return true;
}

bool FSentryOutputDevice::CanBeUsedOnMultipleThreads() const
{
	return true;
}

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
bool FSentryOutputDevice::CanBeUsedOnPanicThread() const
{
	return true;
}
#endif
