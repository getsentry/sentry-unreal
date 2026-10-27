// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "Misc/OutputDevice.h"

#include "SentryDataTypes.h"

class ISentrySubsystem;
class USentrySettings;

class FSentryOutputDevice : public FOutputDevice
{
public:
	FSentryOutputDevice();

	FSentryOutputDevice(const TSharedPtr<ISentrySubsystem>& InNativeImpl, const USentrySettings& Settings);

	static bool IsSerializing();

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	virtual bool CanBeUsedOnAnyThread() const override;
	virtual bool CanBeUsedOnMultipleThreads() const override;

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	virtual bool CanBeUsedOnPanicThread() const override;
#endif

private:
	TSharedPtr<ISentrySubsystem> NativeImpl;

	TMap<ESentryLevel, bool> BreadcrumbFlags;
	TMap<ESentryLevel, bool> StructuredLoggingFlags;

	TArray<FString> StructuredLoggingCategories;
	bool bSendBreadcrumbsWithStructuredLogging;

	void CacheLoggingSettings(const USentrySettings& Settings);

	bool ShouldForwardToStructuredLogging(const FString& Category, ESentryLevel Level) const;
};
