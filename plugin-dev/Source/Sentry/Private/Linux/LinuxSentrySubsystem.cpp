// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "LinuxSentrySubsystem.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentrySettings.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Misc/Paths.h"

#if USE_SENTRY_NATIVE

void FLinuxSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryTraceSampler* TraceSampler)
{
	FGenericPlatformSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);

	InitCrashReporter(Settings->Release, Settings->Environment);
}

void FLinuxSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	sentry_options_set_handler_path(Options, TCHAR_TO_UTF8(*GetHandlerPath()));
}

void FLinuxSentrySubsystem::ConfigureDatabasePath(sentry_options_t* Options)
{
	sentry_options_set_database_path(Options, TCHAR_TO_UTF8(*GetDatabasePath()));
}

void FLinuxSentrySubsystem::ConfigureCertsPath(sentry_options_t* Options)
{
	// In order to use CURL transport for sentry-native we have to manually specify path to a valid CA certificates on Linux.
	// Unreal Engine itself follows a similar approach (see `CertBundlePath` implementation in CurlHttp.cpp for extra details)
	static const char* KnownCertPaths[] = {
		"/etc/pki/tls/certs/ca-bundle.crt",
		"/etc/ssl/certs/ca-certificates.crt",
		"/etc/ssl/ca-bundle.pem"
	};

	for (const char* BundlePath : KnownCertPaths)
	{
		FString FileName(BundlePath);

		if (FPaths::FileExists(FileName))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Sentry transport will use the certificate found at %s for verification."), *FileName);
			sentry_options_set_ca_certs(Options, BundlePath);
			return;
		}
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("Could not find CA certificates in any known location. Sentry transport may not function properly for handled events"));
}

void FLinuxSentrySubsystem::ConfigureLogFileAttachment(sentry_options_t* Options)
{
	const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();
	sentry_options_add_attachment(Options, TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(LogFilePath)));
}

#endif // USE_SENTRY_NATIVE
