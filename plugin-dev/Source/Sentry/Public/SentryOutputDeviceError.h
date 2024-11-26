// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "Misc/OutputDeviceError.h"
#include "Delegates/Delegate.h"

class FSentryOutputDeviceError : public FOutputDeviceError
{
public:
	FSentryOutputDeviceError(FOutputDeviceError* Parent);

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	virtual void HandleError() override;

	FOutputDeviceError* GetParentDevice();

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	TMulticastDelegate<void(const FString&)> OnAssert;
#else
	TMulticastDelegate<void(const FString&), FDefaultTSDelegateUserPolicy> OnAssert;
#endif

private:
	FOutputDeviceError* ParentDevice;
};
