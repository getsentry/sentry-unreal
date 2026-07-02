// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FMacSentrySubsystem : public FGenericPlatformSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers) override;

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureCertsPath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;
	virtual void ConfigureCrashReporterPath(sentry_options_t* Options) override;

	virtual FString GetHandlerExecutableName() const override;
	virtual FString GetCrashReporterExecutableName() const override { return TEXT("Sentry.CrashReporter.app/Contents/MacOS/Sentry.CrashReporter"); }

	virtual bool IsScreenshotSupported() const override { return true; }
	virtual bool IsHangTrackingSupported() const override { return false; }

	virtual FString GetDeviceType() const override { return TEXT("Desktop"); }
};

#else

#include "Apple/AppleSentrySubsystem.h"

#ifdef USE_SENTRY_SESSION_REPLAY
#include "SessionReplay/SentrySessionReplayRecorder.h"
#endif

class FMacSentrySubsystem : public FAppleSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers) override;
	virtual void Close() override;

	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override;

	virtual FString TryCaptureScreenshot() const override;

	virtual FString GetDeviceType() const override { return TEXT("Desktop"); }

protected:
	virtual FString GetGameLogPath() const override;
	virtual FString GetLatestGameLog() const override;

private:
	FDelegateHandle OnHandleSystemErrorDelegateHandle;

#ifdef USE_SENTRY_SESSION_REPLAY
	FString GetReplayPath() const;

	FString SessionReplayId;

	TUniquePtr<FSentrySessionReplayRecorder> SessionReplay;
#endif
};

#endif

typedef FMacSentrySubsystem FPlatformSentrySubsystem;
