// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "Misc/OutputDeviceError.h"
#include "Delegates/Delegate.h"

class FSentryOutputDeviceError : public FOutputDeviceError
{
public:
	FSentryOutputDeviceError(FOutputDeviceError* Parent);

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	virtual void HandleError() override;

	FOutputDeviceError* GetParentDevice();

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
	TMulticastDelegate<void(const FString&), FDefaultTSDelegateUserPolicy> OnAssert;
#else
	TMulticastDelegate<void(const FString&)> OnAssert;
#endif

private:
	FOutputDeviceError* ParentDevice;
};
