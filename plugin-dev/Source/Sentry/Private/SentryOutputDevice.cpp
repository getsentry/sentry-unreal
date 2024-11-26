// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryOutputDevice.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Engine/Engine.h"

void FSentryOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	const FString Message = FString(V).TrimStartAndEnd();
	if (Message.IsEmpty() || Message.StartsWith(TEXT("[Callstack]")))
	{
		return;
	}

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	bool bAddBreadcrumb;

	ESentryLevel BreadcrumbLevel = ESentryLevel::Debug;

	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		bAddBreadcrumb = Settings->AutomaticBreadcrumbsForLogs.bOnFatalLog;
		BreadcrumbLevel = ESentryLevel::Fatal;
		break;
	case ELogVerbosity::Error:
		bAddBreadcrumb = Settings->AutomaticBreadcrumbsForLogs.bOnErrorLog;
		BreadcrumbLevel = ESentryLevel::Error;
		break;
	case ELogVerbosity::Warning:
		bAddBreadcrumb = Settings->AutomaticBreadcrumbsForLogs.bOnWarningLog;
		BreadcrumbLevel = ESentryLevel::Warning;
		break;
	case ELogVerbosity::Display:
	case ELogVerbosity::Log:
		bAddBreadcrumb = Settings->AutomaticBreadcrumbsForLogs.bOnInfoLog;
		BreadcrumbLevel = ESentryLevel::Info;
		break;
	case ELogVerbosity::Verbose:
	case ELogVerbosity::VeryVerbose:
		bAddBreadcrumb = Settings->AutomaticBreadcrumbsForLogs.bOnDebugLog;
		BreadcrumbLevel = ESentryLevel::Debug;
		break;
	default:
		bAddBreadcrumb = false;
	}

	if(!bAddBreadcrumb)
	{
		return;
	}

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if(!SentrySubsystem || !SentrySubsystem->IsEnabled())
	{
		return;
	}

	SentrySubsystem->AddBreadcrumbWithParams(Message, Category.ToString(), FString(), TMap<FString, FString>(), BreadcrumbLevel);
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
