// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/OutputDevice.h"

class FSentryOutputDevice : public FOutputDevice
{
public:
	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	virtual bool CanBeUsedOnAnyThread() const override;
	virtual bool CanBeUsedOnMultipleThreads() const override;
	virtual bool CanBeUsedOnPanicThread() const override;
};
