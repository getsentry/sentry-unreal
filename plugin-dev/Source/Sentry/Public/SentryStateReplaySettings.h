// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "SentryStateReplaySettings.generated.h"

/**
 * Settings for the experimental state-based session replay.
 *
 * Unlike the video-based session replay, this samples a compact, structured
 * snapshot of gameplay state (player/camera transforms, active UI, tracked
 * actors, gameplay events) every frame and streams it to a local newline-
 * delimited JSON (JSONL) file. The format is trivially parseable by tools,
 * AI agents, and the bundled local player.
 *
 * This v0 is a local, developer-facing capture; it does not upload anything.
 */
UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "Sentry State Replay"))
class SENTRY_API USentryStateReplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USentryStateReplaySettings();

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** Master switch for local state-replay capture. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Enable state replay (experimental)",
			ToolTip = "Stream a structured snapshot of gameplay state to a local JSONL file for debugging and AI-assisted analysis."))
	bool bEnabled;

	/** Start a session automatically as soon as a local player exists. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Auto-start session", EditCondition = "bEnabled"))
	bool bAutoStart;

	/** How many state samples to record per second. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Sample rate (Hz)", EditCondition = "bEnabled", ClampMin = "1", ClampMax = "120"))
	int32 SampleRateHz;

	/** Capture the set of active UMG user widgets in each frame. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Capture UI widgets", EditCondition = "bEnabled"))
	bool bCaptureUI;

	/** Directory (relative to the project's Saved dir) where sessions are written. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Output subdirectory", EditCondition = "bEnabled"))
	FString OutputSubdir;

	/** How often (seconds) the JSONL file is flushed to disk. */
	UPROPERTY(Config, EditAnywhere, Category = "State Replay",
		Meta = (DisplayName = "Flush interval (seconds)", EditCondition = "bEnabled", ClampMin = "0.1", ClampMax = "10.0", AdvancedDisplay))
	float FlushIntervalSeconds;
};
