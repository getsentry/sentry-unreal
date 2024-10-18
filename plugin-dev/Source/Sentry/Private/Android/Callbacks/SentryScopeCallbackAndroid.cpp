// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeCallbackAndroid.h"

#include "HAL/CriticalSection.h"

int64 SentryScopeCallbackAndroid::NextDelegateID;
TMap<int64, FSentryScopeDelegate> SentryScopeCallbackAndroid::ScopeDelegates;

FCriticalSection CriticalSection;

int64 SentryScopeCallbackAndroid::SaveDelegate(const FSentryScopeDelegate& onConfigure)
{
	FScopeLock Lock(&CriticalSection);

	int64 delegateId = NextDelegateID++;
	ScopeDelegates.Add(delegateId, onConfigure);

	return delegateId;
}

void SentryScopeCallbackAndroid::RemoveDelegate(int64 delegateId)
{
	FScopeLock Lock(&CriticalSection);

	ScopeDelegates.Remove(delegateId);
}

FSentryScopeDelegate* SentryScopeCallbackAndroid::GetDelegateById(int64 delegateId)
{
	FScopeLock Lock(&CriticalSection);

	return ScopeDelegates.Find(delegateId);
}
