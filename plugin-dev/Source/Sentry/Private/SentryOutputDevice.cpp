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

	StructuredLoggingFlags.Add(ESentryLevel::Fatal, Settings->StructuredLoggingLevels.bOnFatalLog);
	StructuredLoggingFlags.Add(ESentryLevel::Error, Settings->StructuredLoggingLevels.bOnErrorLog);
	StructuredLoggingFlags.Add(ESentryLevel::Warning, Settings->StructuredLoggingLevels.bOnWarningLog);
	StructuredLoggingFlags.Add(ESentryLevel::Info, Settings->StructuredLoggingLevels.bOnInfoLog);
	StructuredLoggingFlags.Add(ESentryLevel::Debug, Settings->StructuredLoggingLevels.bOnDebugLog);

	bIsStructuredLoggingEnabled = Settings->EnableStructuredLogging;
	StructuredLoggingCategories = Settings->StructuredLoggingCategories;
	bSendBreadcrumbsWithStructuredLogging = Settings->bSendBreadcrumbsWithStructuredLogging;
}

void FSentryOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	const FString Message = FString(V).TrimStartAndEnd();
	if (Message.IsEmpty() || Message.Contains(TEXT("[Callstack]")))
	{
		return;
	}

	ESentryLevel Level = SentryLogUtils::ConvertLogVerbosityToSentryLevel(Verbosity);
	const FString CategoryString = Category.ToString();

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!SentrySubsystem || !SentrySubsystem->IsEnabled())
	{
		return;
	}

	if (bIsStructuredLoggingEnabled && ShouldForwardToStructuredLogging(CategoryString, Level))
	{
		// Use level-specific logging methods
		switch (Level)
		{
		case ESentryLevel::Info:
			SentrySubsystem->LogInfo(Message, CategoryString);
			break;
		case ESentryLevel::Warning:
			SentrySubsystem->LogWarning(Message, CategoryString);
			break;
		case ESentryLevel::Error:
			SentrySubsystem->LogError(Message, CategoryString);
			break;
		case ESentryLevel::Fatal:
			SentrySubsystem->LogFatal(Message, CategoryString);
			break;
		case ESentryLevel::Debug:
		default:
			SentrySubsystem->LogDebug(Message, CategoryString);
			break;
		}

		// If we don't want to also send breadcrumbs when structured logging is enabled, return early
		if (!bSendBreadcrumbsWithStructuredLogging)
		{
			return;
		}
	}

	// Send breadcrumb if not sent to structured logging, or if forced to send both
	if (BreadcrumbFlags.Contains(Level) && BreadcrumbFlags[Level])
	{
		SentrySubsystem->AddBreadcrumbWithParams(Message, CategoryString, FString(), TMap<FString, FSentryVariant>(), Level);
	}
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

bool FSentryOutputDevice::ShouldForwardToStructuredLogging(const FString& Category, ESentryLevel Level) const
{
	// Check if this log level should be forwarded
	if (!StructuredLoggingFlags.Contains(Level) || !StructuredLoggingFlags[Level])
	{
		return false;
	}

	// Check category filter
	if (StructuredLoggingCategories.Num() > 0)
	{
		for (const FString& CategoryFilter : StructuredLoggingCategories)
		{
			if (Category.Equals(CategoryFilter, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	// No category filter, forward all logs that passed the level check
	return true;
}
