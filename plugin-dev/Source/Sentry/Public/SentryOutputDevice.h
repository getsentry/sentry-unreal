// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/OutputDevice.h"
#include "Misc/EngineVersionComparison.h"

class FSentryOutputDevice : public FOutputDevice
{
public:
	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	virtual bool CanBeUsedOnAnyThread() const override;
	virtual bool CanBeUsedOnMultipleThreads() const override;

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	virtual bool CanBeUsedOnPanicThread() const override;
#endif
};
