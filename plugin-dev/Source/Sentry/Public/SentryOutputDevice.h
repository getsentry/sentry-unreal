// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/OutputDevice.h"
#include "Runtime/Launch/Resources/Version.h"

class FSentryOutputDevice : public FOutputDevice
{
public:
	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	virtual bool CanBeUsedOnAnyThread() const override;
	virtual bool CanBeUsedOnMultipleThreads() const override;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	virtual bool CanBeUsedOnPanicThread() const override;
#endif
};
