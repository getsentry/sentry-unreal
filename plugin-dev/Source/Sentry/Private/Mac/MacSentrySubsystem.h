#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FMacSentrySubsystem : public FAppleSentrySubsystem
{
public:
	virtual void InitWithSettings(
		const USentrySettings* Settings,
		USentryBeforeSendHandler* BeforeSendHandler,
		USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
		USentryTraceSampler* TraceSampler
	) override;

	virtual void TryCaptureScreenshot() const override;

protected:
	virtual FString GetScreenshotPath() const override;
};

typedef FMacSentrySubsystem FPlatformSentrySubsystem;
