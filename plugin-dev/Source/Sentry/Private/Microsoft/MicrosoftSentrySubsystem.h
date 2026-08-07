// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "MicrosoftSentryCrashLogger.h"

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"
#include "HAL/ThreadSafeBool.h"

class FMicrosoftSentrySubsystem : public FGenericPlatformSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers) override;
	virtual void Close() override;

protected:
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;

	virtual void AddFileAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void AddByteAttachment(TSharedPtr<ISentryAttachment> attachment) override;

	virtual sentry_value_t OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure) override;

	virtual void ConfigureAppHangTracking() override;
	virtual void ResetAppHangTracking() override;
	virtual bool IsAppHangTrackingActive() const override;

private:
	/** Crash logger for safe stack trace logging during crashes */
	TUniquePtr<FMicrosoftSentryCrashLogger> CrashLogger;

	FThreadSafeBool bAppIsActive;
	FThreadSafeBool bAppIsForeground;

	FDelegateHandle AppHangWillDeactivateHandle;
	FDelegateHandle AppHangWillEnterBackgroundHandle;
	FDelegateHandle AppHangHasEnteredForegroundHandle;
	FDelegateHandle AppHangHasReactivatedHandle;
};

#endif // USE_SENTRY_NATIVE
