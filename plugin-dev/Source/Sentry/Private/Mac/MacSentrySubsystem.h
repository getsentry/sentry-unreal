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

	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override;

	virtual FString TryCaptureScreenshot() const override;

private:
	bool isScreenshotAttachmentEnabled = false;
};

typedef FMacSentrySubsystem FPlatformSentrySubsystem;
