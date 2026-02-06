// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "MicrosoftSentryCrashLogger.h"

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FMicrosoftSentrySubsystem : public FGenericPlatformSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryBeforeLogHandler* beforeLogHandler, USentryBeforeMetricHandler* beforeMetricHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;

protected:
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;

	virtual void AddFileAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void AddByteAttachment(TSharedPtr<ISentryAttachment> attachment) override;

	virtual sentry_value_t OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure) override;

private:
	/** Crash logger for safe stack trace logging during crashes */
	TUniquePtr<FMicrosoftSentryCrashLogger> CrashLogger;
};

#endif // USE_SENTRY_NATIVE
