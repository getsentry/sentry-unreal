#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FIOSSentrySubsystem : public FAppleSentrySubsystem
{
public:
	virtual void InitWithSettings(
		const USentrySettings* Settings,
		USentryBeforeSendHandler* BeforeSendHandler,
		USentryTraceSampler* TraceSampler
	) override;

	virtual void TryCaptureScreenshot() const override;

protected:
	virtual FString GetScreenshotPath() const override;
};

typedef FIOSSentrySubsystem FPlatformSentrySubsystem;
