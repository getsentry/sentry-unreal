// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryScopeCallback.h"

int64 AndroidSentryScopeCallback::NextDelegateID;
TMap<int64, FSentryScopeDelegate> AndroidSentryScopeCallback::ScopeDelegates;
FCriticalSection AndroidSentryScopeCallback::CriticalSection;

int64 AndroidSentryScopeCallback::SaveDelegate(const FSentryScopeDelegate& onConfigure)
{
	FScopeLock Lock(&CriticalSection);

	int64 delegateId = NextDelegateID++;
	ScopeDelegates.Add(delegateId, onConfigure);

	return delegateId;
}

void AndroidSentryScopeCallback::RemoveDelegate(int64 delegateId)
{
	FScopeLock Lock(&CriticalSection);

	ScopeDelegates.Remove(delegateId);
}

FSentryScopeDelegate* AndroidSentryScopeCallback::GetDelegateById(int64 delegateId)
{
	FScopeLock Lock(&CriticalSection);

	return ScopeDelegates.Find(delegateId);
}
