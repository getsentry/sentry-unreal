// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

#include "Android/AndroidSentrySubsystem.h"

class AndroidSentryScopeCallback
{
public:
	static int64 SaveDelegate(const FSentryScopeDelegate& onConfigure);
	static void RemoveDelegate(int64 delegateId);
	static FSentryScopeDelegate* GetDelegateById(int64 delegateId);

private:
	static int64 NextDelegateID;
	static TMap<int64, FSentryScopeDelegate> ScopeDelegates;
	static FCriticalSection CriticalSection;
};
