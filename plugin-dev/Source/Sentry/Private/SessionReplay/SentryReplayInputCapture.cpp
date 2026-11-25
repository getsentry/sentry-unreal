// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryReplayInputCapture.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Dom/JsonObject.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformTime.h"
#include "Input/Events.h"
#include "Layout/WidgetPath.h"
#include "Misc/ScopeLock.h"
#include "Widgets/SWindow.h"

namespace
{

// rrweb IncrementalSource
constexpr int32 RRWebMouseMove = 1;
constexpr int32 RRWebMouseInteraction = 2;

// rrweb MouseInteractions
constexpr int32 RRWebClick = 2;
constexpr int32 RRWebTouchStart = 7;
constexpr int32 RRWebTouchEnd = 9;

// rrweb PointerTypes
constexpr int32 RRWebPointerMouse = 0;
constexpr int32 RRWebPointerTouch = 2;

// Same mouse move throttle as rrweb
constexpr double MoveSampleIntervalSeconds = 0.05;

} // namespace

FSentryReplayInputCapture::FSentryReplayInputCapture(double InRetentionSeconds)
	: RetentionSeconds(InRetentionSeconds)
{
}

TArray<FJsonObjectWrapper> FSentryReplayInputCapture::GetInputEvents(double ClipStartSeconds, double ClipEndSeconds, int32 Width, int32 Height) const
{
	TArray<FJsonObjectWrapper> Events;

	FScopeLock Lock(&InputsLock);
	for (const FCapturedInput& Input : Inputs)
	{
		if (Input.TimeSeconds < ClipStartSeconds || Input.TimeSeconds > ClipEndSeconds)
		{
			continue;
		}

		const double X = Input.Position.X * Width;
		const double Y = Input.Position.Y * Height;

		TSharedRef<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetNumberField(TEXT("source"), Input.Source);

		if (Input.Source == RRWebMouseMove)
		{
			TSharedRef<FJsonObject> Position = MakeShared<FJsonObject>();
			Position->SetNumberField(TEXT("x"), X);
			Position->SetNumberField(TEXT("y"), Y);
			Position->SetNumberField(TEXT("id"), 0);
			Position->SetNumberField(TEXT("timeOffset"), 0);
			Data->SetArrayField(TEXT("positions"), { MakeShared<FJsonValueObject>(Position) });
		}
		else
		{
			Data->SetNumberField(TEXT("type"), Input.Type);
			Data->SetNumberField(TEXT("id"), 0);
			Data->SetNumberField(TEXT("x"), X);
			Data->SetNumberField(TEXT("y"), Y);
			Data->SetNumberField(TEXT("pointerType"), Input.PointerType);
			Data->SetNumberField(TEXT("pointerId"), Input.PointerId);
		}

		FJsonObjectWrapper& Event = Events.AddDefaulted_GetRef();
		Event.JsonObject = MakeShared<FJsonObject>();
		Event.JsonObject->SetNumberField(TEXT("type"), 3);
		Event.JsonObject->SetNumberField(TEXT("timestamp"), (Input.TimeSeconds - ClipStartSeconds) * 1000.0);
		Event.JsonObject->SetObjectField(TEXT("data"), Data);
	}

	return Events;
}

bool FSentryReplayInputCapture::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	// A captured mouse (e.g. mouse look) has no meaningful position
	if (MouseEvent.IsTouchEvent() || SlateApp.IsUsingHighPrecisionMouseMovment())
	{
		return false;
	}

	const double Now = FPlatformTime::Seconds();
	if (Now - LastMoveSeconds >= MoveSampleIntervalSeconds && CaptureInput(SlateApp, MouseEvent, RRWebMouseMove, 0, RRWebPointerMouse))
	{
		LastMoveSeconds = Now;
	}

	return false;
}

bool FSentryReplayInputCapture::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	CapturePress(SlateApp, MouseEvent);
	return false;
}

bool FSentryReplayInputCapture::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsTouchEvent())
	{
		CaptureInput(SlateApp, MouseEvent, RRWebMouseInteraction, RRWebTouchEnd, RRWebPointerTouch);
	}
	return false;
}

bool FSentryReplayInputCapture::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	// Slate reports the second press of a double click here instead of as a button down
	CapturePress(SlateApp, MouseEvent);
	return false;
}

void FSentryReplayInputCapture::CapturePress(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	// Slate routes touches through the mouse button path as well
	if (MouseEvent.IsTouchEvent())
	{
		CaptureInput(SlateApp, MouseEvent, RRWebMouseInteraction, RRWebTouchStart, RRWebPointerTouch);
	}
	else
	{
		CaptureInput(SlateApp, MouseEvent, RRWebMouseInteraction, RRWebClick, RRWebPointerMouse);
	}
}

bool FSentryReplayInputCapture::CaptureInput(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, int32 Source, int32 Type, int32 PointerType)
{
	const FVector2D ScreenPosition = MouseEvent.GetScreenSpacePosition();

	const FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(ScreenPosition, SlateApp.GetInteractiveTopLevelWindows());

	// Only regular windows end up in the replay video (see FSentryBackBufferCapture)
	if (!WidgetPath.IsValid() || !WidgetPath.GetWindow()->IsRegularWindow())
	{
		return false;
	}

	const TSharedRef<SWindow> Window = WidgetPath.GetWindow();
	const FVector2D WindowSize = FVector2D(Window->GetSizeInScreen());
	if (WindowSize.X <= 0.0 || WindowSize.Y <= 0.0)
	{
		return false;
	}

	FCapturedInput Input;
	Input.TimeSeconds = FPlatformTime::Seconds();
	Input.Position = (ScreenPosition - FVector2D(Window->GetPositionInScreen())) / WindowSize;
	Input.Source = Source;
	Input.Type = Type;
	Input.PointerType = PointerType;
	Input.PointerId = MouseEvent.IsTouchEvent() ? static_cast<int32>(MouseEvent.GetPointerIndex()) : 0;

	FScopeLock Lock(&InputsLock);

	while (!Inputs.IsEmpty() && Inputs.First().TimeSeconds < Input.TimeSeconds - RetentionSeconds)
	{
		Inputs.PopFront();
	}

	Inputs.Add(Input);

	return true;
}

#endif // USE_SENTRY_SESSION_REPLAY
