// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryId.h"

#include "Interface/SentryIdInterface.h"

FString USentryId::ToString() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->ToString();
}
