// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/OutputDeviceError.h"
#include "Delegates/Delegate.h"

class FSentryOutputDeviceError : public FOutputDeviceError
{
public:
	FSentryOutputDeviceError(FOutputDeviceError* Parent);

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	virtual void HandleError() override;

	FOutputDeviceError* GetParentDevice();

	TMulticastDelegate<void(const FString&), FDefaultTSDelegateUserPolicy> OnAssert;

private:
	FOutputDeviceError* ParentDevice;
};
