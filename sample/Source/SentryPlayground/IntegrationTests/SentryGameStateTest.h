// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class UWorld;

/**
 * Verifies that the captured game state is attached to crash events.
 *
 * Spawns a few tracked actors that move in circles, moves the player pawn and updates
 * game state attributes for several seconds, so the capture has data to record, then crashes.
 *
 * Requires game state capture to be enabled (`AttachGameState=True`).
 */
class FSentryGameStateTest : public FSentryBaseIntegrationTest
{
public:
	FSentryGameStateTest() : FSentryBaseIntegrationTest(TEXT("game-state-capture")) {}

	virtual void Run() override;

private:
	void Tick();
	void SpawnActors(UWorld* World);
	void Crash();

	TArray<TWeakObjectPtr<AActor>> Actors;
	double StartTime = 0.0;
	bool bSpawned = false;
	bool bDestroyedActor = false;
	bool bCrashed = false;
};
