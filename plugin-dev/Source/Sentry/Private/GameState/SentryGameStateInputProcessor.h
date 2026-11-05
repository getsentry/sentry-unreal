// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "Input/Events.h"

DECLARE_DELEGATE_TwoParams(FOnSentryGameStateClick, const FVector2D& /*ScreenPosition*/, const FKey& /*Button*/);

/**
 * Reports mouse button presses before Slate routes them, so clicks on UMG widgets are seen
 * even while the game is paused. Never consumes input.
 */
class FSentryGameStateInputProcessor : public IInputProcessor
{
public:
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		OnClick.ExecuteIfBound(MouseEvent.GetScreenSpacePosition(), MouseEvent.GetEffectingButton());
		return false;
	}

	/** Delegate fired on mouse button press. Called on the game thread. */
	FOnSentryGameStateClick OnClick;
};
