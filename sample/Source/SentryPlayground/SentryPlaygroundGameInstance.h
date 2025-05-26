// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SentryPlaygroundGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SENTRYPLAYGROUND_API USentryPlaygroundGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
};
