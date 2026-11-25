// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "OutputDevice/SentryOutputDevice.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Interface/SentrySubsystemInterface.h"
#include "Utils/SentryLogUtils.h"

#include "Engine/Engine.h"
#include "Templates/UnrealTemplate.h"

static thread_local bool bIsSerializing = false;

FSentryOutputDevice::FSentryOutputDevice()
{
	USentrySubsystem* SentrySubsystem = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (SentrySubsystem)
	{
		NativeImpl = SentrySubsystem->GetNativeObject();
	}

	CacheLoggingSettings(*FSentryModule::Get().GetSettings());
}

FSentryOutputDevice::FSentryOutputDevice(const TSharedPtr<ISentrySubsystem>& InNativeImpl, const USentrySettings& Settings)
	: NativeImpl(InNativeImpl)
{
	CacheLoggingSettings(Settings);
}

bool FSentryOutputDevice::IsSerializing()
{
	return bIsSerializing;
}

void FSentryOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	TGuardValue<bool> IsSerializingGuard(bIsSerializing, true);

	if (!NativeImpl || !NativeImpl->IsEnabled() || NativeImpl->IsCrashing())
	{
		return;
	}

	// Filter internal log messages coming from the underlying platform-specific Sentry SDKs
	if (Category.IsEqual(TEXT("LogSentryInternal")))
	{
		return;
	}

	const FString Message = FString(V).TrimStartAndEnd();
	if (Message.IsEmpty() || SentryLogUtils::IsCallstackLine(Message))
	{
		return;
	}

	const FString CategoryString = Category.ToString();

	const ESentryLogLevel StructuredLogLevel = SentryLogUtils::ConvertLogVerbosityToSentryLogLevel(Verbosity);
	if (ShouldForwardToStructuredLogging(CategoryString, StructuredLogLevel))
	{
		const FString FormattedMessage = FString::Printf(TEXT("[%s] %s"), *CategoryString, *Message);

		TMap<FString, FSentryVariant> Attributes;
		Attributes.Add(TEXT("category"), CategoryString);

		NativeImpl->AddLog(FormattedMessage, StructuredLogLevel, Attributes);

		// If we don't want to also send breadcrumbs when structured logging is enabled, return early
		if (!bAddBreadcrumbsWithStructuredLogs)
		{
			return;
		}
	}

	// Send breadcrumb if not sent to structured logging, or if forced to send both
	const ESentryLevel Level = SentryLogUtils::ConvertLogVerbosityToSentryLevel(Verbosity);
	if (BreadcrumbFlags.Contains(Level) && BreadcrumbFlags[Level])
	{
		NativeImpl->AddBreadcrumbWithParams(Message, CategoryString, FString(), TMap<FString, FSentryVariant>(), Level);
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

void FSentryOutputDevice::CacheLoggingSettings(const USentrySettings& Settings)
{
	BreadcrumbFlags.Add(ESentryLevel::Fatal, Settings.AutomaticBreadcrumbsForLogs.bOnFatalLog);
	BreadcrumbFlags.Add(ESentryLevel::Error, Settings.AutomaticBreadcrumbsForLogs.bOnErrorLog);
	BreadcrumbFlags.Add(ESentryLevel::Warning, Settings.AutomaticBreadcrumbsForLogs.bOnWarningLog);
	BreadcrumbFlags.Add(ESentryLevel::Info, Settings.AutomaticBreadcrumbsForLogs.bOnInfoLog);
	BreadcrumbFlags.Add(ESentryLevel::Debug, Settings.AutomaticBreadcrumbsForLogs.bOnDebugLog);

	StructuredLoggingFlags.Add(ESentryLogLevel::Fatal, Settings.StructuredLogsLevels.bOnFatalLog);
	StructuredLoggingFlags.Add(ESentryLogLevel::Error, Settings.StructuredLogsLevels.bOnErrorLog);
	StructuredLoggingFlags.Add(ESentryLogLevel::Warning, Settings.StructuredLogsLevels.bOnWarningLog);
	StructuredLoggingFlags.Add(ESentryLogLevel::Info, Settings.StructuredLogsLevels.bOnInfoLog);
	StructuredLoggingFlags.Add(ESentryLogLevel::Debug, Settings.StructuredLogsLevels.bOnDebugLog);
	StructuredLoggingFlags.Add(ESentryLogLevel::Trace, Settings.StructuredLogsLevels.bOnTraceLog);

	StructuredLogsCategories = Settings.StructuredLogsCategories;
	bAddBreadcrumbsWithStructuredLogs = Settings.AddBreadcrumbsWithStructuredLogs;
}

bool FSentryOutputDevice::ShouldForwardToStructuredLogging(const FString& Category, ESentryLogLevel Level) const
{
	// Check if this log level should be forwarded
	if (!StructuredLoggingFlags.Contains(Level) || !StructuredLoggingFlags[Level])
	{
		return false;
	}

	// Check category filter
	if (StructuredLogsCategories.Num() > 0)
	{
		for (const FString& CategoryFilter : StructuredLogsCategories)
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
