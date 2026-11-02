// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"

class UWidget;
class UWorld;

class SentryGameStateUtils
{
public:
	/** Converts a record to a UTF-8 line terminated with a newline. */
	static TArray<uint8> ToUtf8Line(const FString& Line);

	static const TCHAR* NetModeToString(ENetMode NetMode);

	static bool IsWidgetVisible(const UWidget* Widget);
	static bool HasArea(const UWidget* Widget);

	/** Returns the viewport size in pixels, at least 1x1. */
	static FVector2D GetViewportSize(UWorld* World);

	/** Converts an absolute position to a viewport position in the 0..1 range. */
	static FVector2D ToNormalizedViewport(UWorld* World, const FVector2D& AbsolutePosition, const FVector2D& ViewportSize);
};
