// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"

#include "SentryStateReplaySubsystem.generated.h"

class AActor;
class UWorld;
class FSentryStateReplayInputProcessor;

/**
 * Records a local, state-based session replay.
 *
 * Each session is a newline-delimited JSON (JSONL) file: a single `header`
 * line followed by `frame` lines (sampled at the configured rate) and `event`
 * lines (pushed by gameplay code or the UI capture). Frames auto-capture the
 * local pawn transform, camera point-of-view, active UMG widgets, the cursor
 * position, and any actors registered via AddTrackedActor.
 *
 * UI interaction is captured at the Slate level via an input pre-processor, so
 * clicks on Blueprint (UMG) menu buttons are recorded even while gameplay is
 * paused and the PlayerController receives no input.
 *
 * The subsystem is engine-agnostic (pure Unreal APIs) and does not touch the
 * crash pipeline; it is a developer/AI debugging tool that runs entirely
 * locally. Product integration (crash-attachment, upload) is intentionally
 * out of scope for this version.
 */
UCLASS()
class SENTRY_API USentryStateReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Convenience accessor from any object with a world context. */
	static USentryStateReplaySubsystem* Get(const UObject* WorldContextObject);

	// UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Begins a new capture session, writing to a fresh JSONL file. No-op if already recording. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void StartSession(const FString& Label);

	/** Ends the current session and flushes the file. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void StopSession();

	UFUNCTION(BlueprintPure, Category = "Sentry|State Replay")
	bool IsRecording() const { return bSessionActive; }

	/** Absolute path of the JSONL file for the active (or most recent) session. */
	UFUNCTION(BlueprintPure, Category = "Sentry|State Replay")
	FString GetSessionFilePath() const { return SessionFilePath; }

	/** Records a semantic gameplay event (e.g. "combat"/"turret_fire") into the stream. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void AddEvent(const FString& Category, const FString& Name, const TMap<FString, FString>& Data);

	/** Starts sampling an actor's transform into every frame under a stable id/tag. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag);

	/** Stops sampling a previously tracked actor. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void RemoveTrackedActor(AActor* Actor);

	/** Sets a numeric game-state channel sampled into every frame's `state` (e.g. "score", "wave", "health"). */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void SetState(const FString& Key, float Value);

	/** Sets a string game-state channel sampled into every frame's `state`. */
	UFUNCTION(BlueprintCallable, Category = "Sentry|State Replay")
	void SetStateString(const FString& Key, const FString& Value);

	/** Called by the Slate input pre-processor on a mouse-button press (any widget, incl. UMG). */
	void OnUIClick(const FVector2D& ScreenPos, const FString& Button);

private:
	struct FTrackedActor
	{
		TWeakObjectPtr<AActor> Actor;
		FString Id;
		FString Tag;
	};

	bool Tick(float DeltaTime);
	UWorld* ResolveGameWorld() const;
	void SampleFrame(UWorld* World, double SessionTime);
	void CaptureUI(UWorld* World, FString& OutUiArray);
	FString ResolveWidgetAt(const FVector2D& ScreenPos, UWorld* World) const;
	void WriteLine(const FString& Line);
	void WriteLineUnlocked(const FString& Line);
	double Now() const;

	// Config snapshot (read once at Initialize)
	bool bEnabled = false;
	bool bAutoStart = true;
	bool bCaptureUI = true;
	int32 SampleRateHz = 10;
	FString OutputSubdir;
	float FlushIntervalSeconds = 1.0f;

	// Runtime state (game thread)
	FTSTicker::FDelegateHandle TickerHandle;
	bool bSessionActive = false;
	double SessionStartSeconds = 0.0;
	double LastFlushSeconds = 0.0;
	FString SessionFilePath;
	FString CurrentMapName;
	TArray<FTrackedActor> TrackedActors;
	TMap<FString, float> NumericState;
	TMap<FString, FString> TextState;
	TSet<FString> ActiveTopWidgets;
	TSharedPtr<FSentryStateReplayInputProcessor> InputProcessor;

	// Guards the file writer + event/tracked mutations (writes may arrive from
	// gameplay code; sampling runs on the game-thread ticker).
	FCriticalSection WriterLock;
	TUniquePtr<FArchive> FileWriter;
};
