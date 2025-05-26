// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeBreadcrumbHandler.generated.h"

class USentryBreadcrumb;
class USentryHint;

UCLASS(Abstract, Blueprintable)
class SENTRY_API USentryBeforeBreadcrumbHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryBreadcrumb* HandleBeforeBreadcrumb(USentryBreadcrumb* Breadcrumb, USentryHint* Hint);
	virtual USentryBreadcrumb* HandleBeforeBreadcrumb_Implementation(USentryBreadcrumb* Breadcrumb, USentryHint* Hint);
};