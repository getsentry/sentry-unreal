// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryScope.h"

#include "SentryScopeCallbackAndroid.generated.h"

UCLASS()
class USentryScopeCallbackAndroid : public UObject
{
	GENERATED_BODY()

public:
	USentryScopeCallbackAndroid();

	void BindDelegate(const FConfigureScopeDelegate& OnConfigure);
	void ExecuteDelegate(USentryScope* Scope);

private:
	UPROPERTY()
	FConfigureScopeDelegate OnConfigureDelegate;
};
