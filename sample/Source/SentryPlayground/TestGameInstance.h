// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TestGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SENTRYPLAYGROUND_API UTestGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
};
