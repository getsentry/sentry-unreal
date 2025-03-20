// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryId.h"

#include "HAL/PlatformSentryId.h"

FSentryId::FSentryId()
{
}

FSentryId::FSentryId(TSharedPtr<ISentryId> Id)
{
	NativeImpl = Id;
}

FString FSentryId::ToString() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->ToString();
}
