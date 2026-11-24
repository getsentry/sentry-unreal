// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeBreadcrumbHandler.h"

#include "CppBeforeBreadcrumbHandler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppBeforeBreadcrumbHandler : public USentryBeforeBreadcrumbHandler
{
	GENERATED_BODY()

public:
	virtual USentryBreadcrumb* HandleBeforeBreadcrumb_Implementation(USentryBreadcrumb* Breadcrumb, USentryHint* Hint) override;
};
