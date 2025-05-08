#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FIOSSentrySubsystem : public FAppleSentrySubsystem
{
public:
	virtual void InitWithSettings(
		const USentrySettings* Settings,
		USentryBeforeSendHandler* BeforeSendHandler,
		USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
		USentryTraceSampler* TraceSampler
	) override;

	virtual FString TryCaptureScreenshot() const override;

protected:
	virtual FString GetGameLogPath() const override;
	virtual FString GetLatestGameLog() const override;

private:
	FString NormalizeToPublicIOSPath(const FString& logFilePath) const;
};

typedef FIOSSentrySubsystem FPlatformSentrySubsystem;
