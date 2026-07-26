// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryPlaygroundTestUtils.generated.h"

UCLASS()
class USentryPlaygroundTestUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Caps the amount of memory the process can commit at current usage plus the given headroom.
	 * Allocations beyond the limit fail as if the machine ran out of memory, while the rest
	 * of the system (including out-of-process crash handlers) keeps its memory headroom.
	 * Windows only - no-op on other platforms.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetMemoryLimit(int32 HeadroomMB);
};
