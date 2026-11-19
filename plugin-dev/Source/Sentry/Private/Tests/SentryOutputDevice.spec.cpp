// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryOutputDevice.h"
#include "SentrySettings.h"
#include "SentryTests.h"

#include "Null/NullSentrySubsystem.h"
#include "Utils/SentryLogUtils.h"

#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

#if WITH_AUTOMATION_TESTS

class FSentryOutputDeviceTestSubsystem : public FNullSentrySubsystem
{
public:
	virtual bool IsEnabled() override { return true; }

	virtual void AddLog(const FString& Message, ESentryLogLevel Level, const TMap<FString, FSentryVariant>& Attributes) override
	{
		LogLevels.Add(Level);
	}

	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level) override
	{
		BreadcrumbLevels.Add(Level);
	}

	TArray<ESentryLogLevel> LogLevels;
	TArray<ESentryLevel> BreadcrumbLevels;
};

BEGIN_DEFINE_SPEC(SentryOutputDeviceSpec, "Sentry.SentryOutputDevice", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	TSharedPtr<FSentryOutputDeviceTestSubsystem> TestSubsystem;
	USentrySettings* Settings;

	void SerializeMessage(ELogVerbosity::Type Verbosity);
END_DEFINE_SPEC(SentryOutputDeviceSpec)

void SentryOutputDeviceSpec::SerializeMessage(ELogVerbosity::Type Verbosity)
{
	FSentryOutputDevice OutputDevice(TestSubsystem, *Settings);
	OutputDevice.Serialize(TEXT("Test message"), Verbosity, TEXT("LogSentryTest"));
}

void SentryOutputDeviceSpec::Define()
{
	BeforeEach([this]()
	{
		TestSubsystem = MakeShared<FSentryOutputDeviceTestSubsystem>();

		Settings = NewObject<USentrySettings>(GetTransientPackage());
		Settings->StructuredLoggingCategories.Empty();
		Settings->StructuredLoggingLevels = FStructuredLoggingLevels();
		Settings->AutomaticBreadcrumbsForLogs = FAutomaticBreadcrumbsForLogs();
		Settings->bSendBreadcrumbsWithStructuredLogging = false;
	});

	AfterEach([this]()
	{
		TestSubsystem.Reset();
		Settings = nullptr;
	});

	Describe("Structured logging", [this]()
	{
		It("should map each verbosity to its log level", [this]()
		{
			Settings->StructuredLoggingLevels.bOnFatalLog = true;
			Settings->StructuredLoggingLevels.bOnErrorLog = true;
			Settings->StructuredLoggingLevels.bOnWarningLog = true;
			Settings->StructuredLoggingLevels.bOnInfoLog = true;
			Settings->StructuredLoggingLevels.bOnDebugLog = true;
			Settings->StructuredLoggingLevels.bOnTraceLog = true;

			SerializeMessage(ELogVerbosity::Fatal);
			SerializeMessage(ELogVerbosity::Error);
			SerializeMessage(ELogVerbosity::Warning);
			SerializeMessage(ELogVerbosity::Display);
			SerializeMessage(ELogVerbosity::Log);
			SerializeMessage(ELogVerbosity::Verbose);
			SerializeMessage(ELogVerbosity::VeryVerbose);

			const TArray<ESentryLogLevel> ExpectedLevels = {
				ESentryLogLevel::Fatal,
				ESentryLogLevel::Error,
				ESentryLogLevel::Warning,
				ESentryLogLevel::Info,
				ESentryLogLevel::Info,
				ESentryLogLevel::Debug,
				ESentryLogLevel::Trace
			};

			TestTrue("Forwarded log levels", TestSubsystem->LogLevels == ExpectedLevels);
		});

		It("should forward VeryVerbose messages only when the trace flag is set", [this]()
		{
			Settings->StructuredLoggingLevels.bOnDebugLog = true;

			SerializeMessage(ELogVerbosity::Verbose);
			SerializeMessage(ELogVerbosity::VeryVerbose);

			TestTrue("Forwarded log levels", TestSubsystem->LogLevels == TArray<ESentryLogLevel>({ ESentryLogLevel::Debug }));
		});

		It("should forward Verbose messages only when the debug flag is set", [this]()
		{
			Settings->StructuredLoggingLevels.bOnTraceLog = true;

			SerializeMessage(ELogVerbosity::Verbose);
			SerializeMessage(ELogVerbosity::VeryVerbose);

			TestTrue("Forwarded log levels", TestSubsystem->LogLevels == TArray<ESentryLogLevel>({ ESentryLogLevel::Trace }));
		});
	});

	Describe("Breadcrumbs", [this]()
	{
		It("should add VeryVerbose messages as debug breadcrumbs", [this]()
		{
			Settings->AutomaticBreadcrumbsForLogs.bOnDebugLog = true;

			SerializeMessage(ELogVerbosity::Verbose);
			SerializeMessage(ELogVerbosity::VeryVerbose);

			TestTrue("Breadcrumb levels", TestSubsystem->BreadcrumbLevels == TArray<ESentryLevel>({ ESentryLevel::Debug, ESentryLevel::Debug }));
			TestEqual("Forwarded log count", TestSubsystem->LogLevels.Num(), 0);
		});
	});

	Describe("Legacy structured logging config", [this]()
	{
		It("should keep forwarding VeryVerbose messages when only the debug flag is configured", [this]()
		{
			Settings->StructuredLoggingLevels.bOnDebugLog = true;

			SentryLogUtils::MigrateStructuredLoggingLevels(Settings->StructuredLoggingLevels,
				TEXT("(bOnFatalLog=False,bOnErrorLog=False,bOnWarningLog=False,bOnInfoLog=False,bOnDebugLog=True)"));

			TestTrue("Trace flag", Settings->StructuredLoggingLevels.bOnTraceLog);

			SerializeMessage(ELogVerbosity::Verbose);
			SerializeMessage(ELogVerbosity::VeryVerbose);

			TestTrue("Forwarded log levels", TestSubsystem->LogLevels == TArray<ESentryLogLevel>({ ESentryLogLevel::Debug, ESentryLogLevel::Trace }));
		});

		It("should keep the trace flag disabled when the debug flag is disabled", [this]()
		{
			SentryLogUtils::MigrateStructuredLoggingLevels(Settings->StructuredLoggingLevels, TEXT("(bOnErrorLog=True,bOnDebugLog=False)"));

			TestFalse("Trace flag", Settings->StructuredLoggingLevels.bOnTraceLog);
		});

		It("should not override an explicitly configured trace flag", [this]()
		{
			Settings->StructuredLoggingLevels.bOnDebugLog = true;

			SentryLogUtils::MigrateStructuredLoggingLevels(Settings->StructuredLoggingLevels, TEXT("(bOnDebugLog=True,bOnTraceLog=False)"));

			TestFalse("Trace flag", Settings->StructuredLoggingLevels.bOnTraceLog);
		});
	});
}

#endif
