// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryId.generated.h"

class ISentryId;

UCLASS(BlueprintType)
class SENTRYSDK_API USentryId : public UObject
{
	GENERATED_BODY()

public:
	USentryId();

	void InitWithNativeImpl(TSharedPtr<ISentryId> idImpl);
	TSharedPtr<ISentryId> GetNativeImpl();

private:
	TSharedPtr<ISentryId> SentryIdNativeImpl;
};
