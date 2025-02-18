// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryId.h"

#include "HAL/PlatformSentryId.h"

void USentryId::Initialize()
{
	NativeImpl = CreateSharedSentryId();
}

FString USentryId::ToString() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->ToString();
}
