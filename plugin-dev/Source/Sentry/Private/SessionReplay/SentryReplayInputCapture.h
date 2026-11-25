// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Containers/RingBuffer.h"
#include "Framework/Application/IInputProcessor.h"
#include "HAL/CriticalSection.h"
#include "JsonObjectWrapper.h"

class FSlateApplication;
struct FPointerEvent;

/**
 * Captures mouse and touch input over game windows so the replay player can show it on top of the video.
 * Runs as a Slate input pre-processor on the game thread and never consumes input.
 */
class FSentryReplayInputCapture : public IInputProcessor
{
public:
	explicit FSentryReplayInputCapture(double InRetentionSeconds);

	// rrweb events for the input within the clip, timed in ms from ClipStartSeconds and positioned in video pixels
	TArray<FJsonObjectWrapper> GetInputEvents(double ClipStartSeconds, double ClipEndSeconds, int32 Width, int32 Height) const;

	// IInputProcessor
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

private:
	struct FCapturedInput
	{
		// FPlatformTime::Seconds() when the input happened
		double TimeSeconds = 0.0;

		// Position within the window, in [0, 1]
		FVector2D Position = FVector2D::ZeroVector;

		int32 Source = 0;
		int32 Type = 0;
		int32 PointerType = 0;
		int32 PointerId = 0;
	};

	void CapturePress(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	bool CaptureInput(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, int32 Source, int32 Type, int32 PointerType);

	double RetentionSeconds;

	// Game thread only
	double LastMoveSeconds = 0.0;

	// Oldest first, protected by InputsLock
	mutable FCriticalSection InputsLock;
	TRingBuffer<FCapturedInput> Inputs;
};

#endif // USE_SENTRY_SESSION_REPLAY
