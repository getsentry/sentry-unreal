// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_IOS
@class SentryScope;
#endif

#include "SentryDataTypes.h"

#include "SentryScope.generated.h"

UCLASS(BlueprintType)
class SENTRYSDK_API USentryScope : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel level);

#if PLATFORM_IOS
	void InitWithNativeObjectIOS(SentryScope* scope);
#endif

private:
#if PLATFORM_IOS
	SentryScope* _scopeIOS;
#endif
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureScopeDelegate, USentryScope*, Scope);
