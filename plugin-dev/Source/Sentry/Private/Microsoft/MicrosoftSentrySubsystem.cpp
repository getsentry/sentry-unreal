#include "MicrosoftSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "Misc/Paths.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"

void FMicrosoftSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	if (!FSentryModule::Get().IsMarketplaceVersion())
	{
		const FString HandlerPath = GetHandlerPath();

		if (!FPaths::FileExists(HandlerPath))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Crashpad executable couldn't be found so Breakpad will be used instead. "
				"Please make sure that the plugin was rebuilt to avoid initialization failure."));
		}

		sentry_options_set_handler_pathw(Options, *HandlerPath);
	}
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

void FMicrosoftSentrySubsystem::ConfigureScreenshotAttachment(sentry_options_t* Options)
{
	sentry_options_add_attachmentw(Options, *GetScreenshotPath());
}

#endif // USE_SENTRY_NATIVE
