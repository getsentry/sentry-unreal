// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "Misc/OutputDevice.h"

#include "SentryDataTypes.h"

class FSentryOutputDevice : public FOutputDevice
{
public:
	FSentryOutputDevice();

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	virtual bool CanBeUsedOnAnyThread() const override;
	virtual bool CanBeUsedOnMultipleThreads() const override;

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	virtual bool CanBeUsedOnPanicThread() const override;
#endif

private:
	TMap<ESentryLevel, bool> BreadcrumbFlags;
};
