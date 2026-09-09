// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryToolset.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "ToolsetRegistry/ToolCallExceptionHandler.h"
#include "ToolsetRegistry/UToolsetRegistry.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryToolsetSpec, "Sentry.SentryToolset", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	// Tool methods raise script errors, which only surface inside a Blueprint stack frame. Run the
	// call through the handler so a direct C++ invocation reports them.
	FString CaptureException(TFunction<void()> && Func)
	{
		UE::ToolsetRegistry::FToolCallExceptionHandler Handler;
		Handler.CaptureErrorsIn(MoveTemp(Func));
		return Handler.GetException();
	}
END_DEFINE_SPEC(SentryToolsetSpec)

void SentryToolsetSpec::Define()
{
	Describe("Toolset registration", [this]()
	{
		It("should expose the toolset to the registry", [this]()
		{
			TestTrue(TEXT("Toolset is registered"), UToolsetRegistry::IsToolsetClassRegistered(USentryToolset::StaticClass()));
		});

		It("should advertise its tools in the schema", [this]()
		{
			const FString Schema = UToolsetRegistry::GetToolsetJsonSchema(USentryToolset::StaticClass());
			TestTrue(TEXT("Schema contains IsSentryEnabled"), Schema.Contains(TEXT("IsSentryEnabled")));
			TestTrue(TEXT("Schema contains IsDsnConfigured"), Schema.Contains(TEXT("IsDsnConfigured")));
			TestTrue(TEXT("Schema contains SetDsn"), Schema.Contains(TEXT("SetDsn")));
			TestTrue(TEXT("Schema contains ReinitializeSentry"), Schema.Contains(TEXT("ReinitializeSentry")));
			TestTrue(TEXT("Schema contains CaptureTestMessage"), Schema.Contains(TEXT("CaptureTestMessage")));
		});
	});

	Describe("IsSentryEnabled", [this]()
	{
		It("should report the state of the Sentry subsystem", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
			TestNotNull(TEXT("Sentry subsystem"), SentrySubsystem);

			TestEqual(TEXT("Reported state"), USentryToolset::IsSentryEnabled(), SentrySubsystem->IsEnabled());
		});

		It("should report the SDK as not running once it's closed", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
			TestNotNull(TEXT("Sentry subsystem"), SentrySubsystem);

			const bool bWasEnabled = SentrySubsystem->IsEnabled();
			SentrySubsystem->Close();

			TestFalse(TEXT("Reported as running"), USentryToolset::IsSentryEnabled());

			if (bWasEnabled)
			{
				SentrySubsystem->Initialize();
			}
		});
	});

	Describe("IsDsnConfigured", [this]()
	{
		It("should report whether the settings carry a DSN", [this]()
		{
			const USentrySettings* Settings = FSentryModule::Get().GetSettings();
			TestNotNull(TEXT("Sentry settings"), Settings);

			TestEqual(TEXT("Reported state"), USentryToolset::IsDsnConfigured(), !Settings->Dsn.IsEmpty());
		});
	});

	Describe("SetDsn", [this]()
	{
		It("should raise when the DSN is empty", [this]()
		{
			const USentrySettings* Settings = FSentryModule::Get().GetSettings();
			TestNotNull(TEXT("Sentry settings"), Settings);
			const FString DsnBefore = Settings->Dsn;

			const FString Exception = CaptureException([]()
			{
				USentryToolset::SetDsn(FString());
			});

			TestTrue(TEXT("Raised the expected error"), Exception.Contains(TEXT("Dsn must not be empty")));
			TestEqual(TEXT("DSN is unchanged"), Settings->Dsn, DsnBefore);
		});

		// SetDsn persists to the project config, which tests must not modify. Exercise the underlying
		// single-property write against a throwaway file instead, so a silent no-op would still fail.
		It("should persist the DSN property to the config file it targets", [this]()
		{
			USentrySettings* Settings = FSentryModule::Get().GetSettings();
			TestNotNull(TEXT("Sentry settings"), Settings);

			const FProperty* DsnProperty = USentrySettings::StaticClass()->FindPropertyByName(
				GET_MEMBER_NAME_CHECKED(USentrySettings, Dsn));
			TestNotNull(TEXT("Dsn property"), DsnProperty);

			const FString ScratchIni = FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("SentryToolsetSpec.ini"));
			IFileManager::Get().Delete(*ScratchIni);

			const FString DsnBefore = Settings->Dsn;
			const FString TestDsn = TEXT("https://examplePublicKey@o0.ingest.sentry.io/0");

			Settings->Dsn = TestDsn;
			Settings->UpdateSinglePropertyInConfigFile(DsnProperty, ScratchIni);
			Settings->Dsn = DsnBefore;

			FString PersistedDsn;
			GConfig->GetString(*USentrySettings::StaticClass()->GetPathName(), TEXT("Dsn"), PersistedDsn, ScratchIni);
			TestEqual(TEXT("Persisted DSN"), PersistedDsn, TestDsn);

			GConfig->UnloadFile(ScratchIni);
			IFileManager::Get().Delete(*ScratchIni);
		});
	});

	Describe("ReinitializeSentry", [this]()
	{
		It("should report a state that agrees with IsSentryEnabled", [this]()
		{
			const bool bEnabled = USentryToolset::ReinitializeSentry();

			TestEqual(TEXT("Agrees with IsSentryEnabled"), bEnabled, USentryToolset::IsSentryEnabled());
		});

		It("should bring the SDK back up after it was closed", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
			TestNotNull(TEXT("Sentry subsystem"), SentrySubsystem);

			// Only meaningful when the project is configured; without a DSN the SDK cannot start.
			if (!USentryToolset::IsDsnConfigured())
			{
				return;
			}

			SentrySubsystem->Close();
			TestFalse(TEXT("Closed"), USentryToolset::IsSentryEnabled());

			TestTrue(TEXT("Running after reinitialization"), USentryToolset::ReinitializeSentry());
		});
	});

	Describe("CaptureTestMessage", [this]()
	{
		It("should raise when the message is empty", [this]()
		{
			const FString Exception = CaptureException([]()
			{
				USentryToolset::CaptureTestMessage(FString());
			});

			TestTrue(TEXT("Raised the expected error"), Exception.Contains(TEXT("Message must not be empty")));
		});

		It("should raise when the SDK isn't running", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
			TestNotNull(TEXT("Sentry subsystem"), SentrySubsystem);

			const bool bWasEnabled = SentrySubsystem->IsEnabled();
			SentrySubsystem->Close();

			const FString Exception = CaptureException([]()
			{
				USentryToolset::CaptureTestMessage(TEXT("Test message"));
			});

			TestTrue(TEXT("Raised the expected error"), Exception.Contains(TEXT("Sentry SDK is not running")));

			if (bWasEnabled)
			{
				SentrySubsystem->Initialize();
			}
		});
	});
}

#endif
