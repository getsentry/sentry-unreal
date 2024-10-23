// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Android/SentrySubsystemAndroid.h"

class SentryScopeCallbackAndroid
{
public:
	static int64 SaveDelegate(const FSentryScopeDelegate& onConfigure);
	static void RemoveDelegate(int64 delegateId);
	static FSentryScopeDelegate* GetDelegateById(int64 delegateId);

private:
	static int64 NextDelegateID;
	static TMap<int64, FSentryScopeDelegate> ScopeDelegates;
};
