// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FSentryGameStateRecorder;
class UWorld;

/**
 * Turns the state of a game world into frame records and writes game state events.
 *
 * Must be used from the game thread.
 */
class FSentryGameStateSampler
{
public:
	/** Writes a frame record with the current state of the given world. */
	void SampleFrame(UWorld* World, double Time, FSentryGameStateRecorder& Recorder);

	/** Clears all state collected so far. */
	void Reset();

	/** Writes a map change event with the name of the new map. */
	void WriteMapChangedEvent(UWorld* World, double Time, FSentryGameStateRecorder& Recorder) const;

private:
	void WriteEvent(double Time, const TCHAR* Category, const TCHAR* Name, const TMap<FString, FString>& Data, FSentryGameStateRecorder& Recorder) const;
};
