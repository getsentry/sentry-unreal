#include "LinuxSentrySubsystem.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "Misc/Paths.h"

#if USE_SENTRY_NATIVE

void FLinuxSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	sentry_options_set_handler_path(Options, TCHAR_TO_UTF8(*GetHandlerPath()));
}

void FLinuxSentrySubsystem::ConfigureDatabasePath(sentry_options_t* Options)
{
	sentry_options_set_database_path(Options, TCHAR_TO_UTF8(*GetDatabasePath()));
}

void FLinuxSentrySubsystem::ConfigureLogFileAttachment(sentry_options_t* Options)
{
	const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();
	sentry_options_add_attachment(Options, TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(LogFilePath)));
}

void FLinuxSentrySubsystem::ConfigureScreenshotAttachment(sentry_options_t* Options)
{
	sentry_options_add_attachment(Options, TCHAR_TO_UTF8(*GetScreenshotPath()));
}

void FLinuxSentrySubsystem::ConfigureGpuDumpAttachment(sentry_options_t* Options)
{
	sentry_options_add_attachment(Options, TCHAR_TO_UTF8(*GetGpuDumpBackupPath()));
}

#endif // USE_SENTRY_NATIVE
