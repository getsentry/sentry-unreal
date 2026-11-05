// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "SentryVariant.h"

class AActor;
class FSentryGameStateJsonWriter;
class FSentryGameStateRecorder;
class UUserWidget;
class UWorld;
struct FKey;

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

	/** Starts recording the actor's transform in every frame. Returns false if the actor is already tracked. */
	bool AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag);
	void RemoveTrackedActor(AActor* Actor);

	void SetAttribute(const FString& Key, const FSentryVariant& Value);
	void RemoveAttribute(const FString& Key);

	/** Enables recording of UMG widgets in the viewport. */
	void SetUIOptions(bool bInCaptureUI, bool bInCaptureUIText);

	/** Clears tracked actors, attributes and UI state. */
	void Reset();

	/** Writes a track event for every tracked actor, e.g. for those tracked before the capture began. */
	void WriteInitialTrackEvents(double Time, FSentryGameStateRecorder& Recorder) const;

	/** Writes a track or untrack event for the actor. Does nothing if the actor isn't tracked. */
	void WriteTrackEvent(AActor* Actor, double Time, FSentryGameStateRecorder& Recorder) const;
	void WriteUntrackEvent(AActor* Actor, double Time, FSentryGameStateRecorder& Recorder) const;

	/** Writes a map change event with the name of the new map. */
	void WriteMapChangedEvent(UWorld* World, double Time, FSentryGameStateRecorder& Recorder) const;

	/** Writes a click event with the widget under the cursor. */
	void WriteClickEvent(UWorld* World, double Time, const FVector2D& ScreenPosition, const FKey& Button, FSentryGameStateRecorder& Recorder) const;

private:
	static constexpr int32 MaxWidgets = 256;
	static constexpr int32 MaxTextLength = 120;

	struct FTrackedActor
	{
		TWeakObjectPtr<AActor> Actor;
		FString Id;
		FString Tag;
	};

	const FTrackedActor* FindTrackedActor(AActor* Actor) const;

	void WriteActorEvent(const FTrackedActor& Tracked, double Time, const TCHAR* Name, FSentryGameStateRecorder& Recorder) const;
	void WriteEvent(double Time, const TCHAR* Category, const TCHAR* Name, const TMap<FString, FString>& Data, FSentryGameStateRecorder& Recorder) const;

	void WriteUI(UWorld* World, double Time, FSentryGameStateJsonWriter& Writer, FSentryGameStateRecorder& Recorder);

	FString FindWidgetAt(const FVector2D& ScreenPosition) const;

	TArray<FTrackedActor> TrackedActors;
	TMap<FString, FSentryVariant> Attributes;

	bool bCaptureUI = false;
	bool bCaptureUIText = false;

	TArray<TWeakObjectPtr<UUserWidget>> ViewportWidgets;
	TSet<FString> OpenWidgets;
};
