// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateSampler.h"

#include "GameState/SentryGameStateJsonWriter.h"
#include "GameState/SentryGameStateRecorder.h"
#include "GameState/SentryGameStateUtils.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/App.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectIterator.h"

void FSentryGameStateSampler::SampleFrame(UWorld* World, double Time, FSentryGameStateRecorder& Recorder)
{
	FSentryGameStateJsonWriter Writer;
	Writer.BeginObject()
		.Write(TEXT("type"), TEXT("frame"))
		.Write(TEXT("t"), Time);

	const double DeltaTime = FApp::GetDeltaTime();
	Writer.BeginObject(TEXT("perf"))
		.Write(TEXT("fps"), DeltaTime > 0.0 ? 1.0 / DeltaTime : 0.0, 1)
		.Write(TEXT("ms"), DeltaTime * 1000.0, 2)
		.EndObject();

	if (World->IsPaused())
	{
		Writer.Write(TEXT("paused"), true);
	}

	if (const APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		if (const APawn* Pawn = PlayerController->GetPawn())
		{
			Writer.BeginObject(TEXT("player"))
				.Write(TEXT("loc"), Pawn->GetActorLocation())
				.Write(TEXT("rot"), Pawn->GetActorRotation())
				.EndObject();
		}

		if (const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			Writer.BeginObject(TEXT("camera"))
				.Write(TEXT("loc"), CameraManager->GetCameraLocation())
				.Write(TEXT("rot"), CameraManager->GetCameraRotation())
				.Write(TEXT("fov"), static_cast<double>(CameraManager->GetFOVAngle()), 1)
				.EndObject();
		}
	}

	TArray<FTrackedActor> DestroyedActors;

	if (TrackedActors.Num() > 0)
	{
		Writer.BeginArray(TEXT("actors"));

		for (int32 Index = 0; Index < TrackedActors.Num();)
		{
			const FTrackedActor& Tracked = TrackedActors[Index];

			const AActor* Actor = Tracked.Actor.Get();
			if (!Actor)
			{
				DestroyedActors.Add(Tracked);
				TrackedActors.RemoveAt(Index);
				continue;
			}

			Writer.BeginObject()
				.Write(TEXT("id"), Tracked.Id)
				.Write(TEXT("tag"), Tracked.Tag)
				.Write(TEXT("loc"), Actor->GetActorLocation())
				.Write(TEXT("rot"), Actor->GetActorRotation())
				.EndObject();

			++Index;
		}

		Writer.EndArray();
	}

	if (Attributes.Num() > 0)
	{
		Writer.BeginObject(TEXT("attributes"));

		for (const TPair<FString, FSentryVariant>& Attribute : Attributes)
		{
			Writer.Write(*Attribute.Key, Attribute.Value);
		}

		Writer.EndObject();
	}

	if (bCaptureUI && FSlateApplication::IsInitialized())
	{
		WriteUI(World, Time, Writer, Recorder);
	}

	Writer.EndObject();

	for (const FTrackedActor& Destroyed : DestroyedActors)
	{
		WriteActorEvent(Destroyed, Time, TEXT("destroyed"), Recorder);
	}

	Recorder.AddRecord(Time, Writer.GetOutput());
}

bool FSentryGameStateSampler::AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag)
{
	if (FindTrackedActor(Actor))
	{
		return false;
	}

	TrackedActors.Add({ Actor, Id, Tag });
	return true;
}

void FSentryGameStateSampler::RemoveTrackedActor(AActor* Actor)
{
	TrackedActors.RemoveAll([Actor](const FTrackedActor& Tracked)
	{
		return Tracked.Actor.Get() == Actor;
	});
}

void FSentryGameStateSampler::SetAttribute(const FString& Key, const FSentryVariant& Value)
{
	Attributes.Add(Key, Value);
}

void FSentryGameStateSampler::RemoveAttribute(const FString& Key)
{
	Attributes.Remove(Key);
}

void FSentryGameStateSampler::SetUIOptions(bool bInCaptureUI, bool bInCaptureUIText)
{
	bCaptureUI = bInCaptureUI;
	bCaptureUIText = bInCaptureUIText;
}

void FSentryGameStateSampler::Reset()
{
	TrackedActors.Reset();
	Attributes.Reset();
	ViewportWidgets.Reset();
	OpenWidgets.Reset();
}

void FSentryGameStateSampler::WriteInitialTrackEvents(double Time, FSentryGameStateRecorder& Recorder) const
{
	for (const FTrackedActor& Tracked : TrackedActors)
	{
		WriteActorEvent(Tracked, Time, TEXT("track"), Recorder);
	}
}

void FSentryGameStateSampler::WriteTrackEvent(AActor* Actor, double Time, FSentryGameStateRecorder& Recorder) const
{
	if (const FTrackedActor* Tracked = FindTrackedActor(Actor))
	{
		WriteActorEvent(*Tracked, Time, TEXT("track"), Recorder);
	}
}

void FSentryGameStateSampler::WriteUntrackEvent(AActor* Actor, double Time, FSentryGameStateRecorder& Recorder) const
{
	if (const FTrackedActor* Tracked = FindTrackedActor(Actor))
	{
		WriteActorEvent(*Tracked, Time, TEXT("untrack"), Recorder);
	}
}

void FSentryGameStateSampler::WriteMapChangedEvent(UWorld* World, double Time, FSentryGameStateRecorder& Recorder) const
{
	WriteEvent(Time, TEXT("world"), TEXT("map_changed"), { { TEXT("map"), UWorld::RemovePIEPrefix(World->GetMapName()) } }, Recorder);
}

void FSentryGameStateSampler::WriteClickEvent(UWorld* World, double Time, const FVector2D& ScreenPosition, const FKey& Button, FSentryGameStateRecorder& Recorder) const
{
	const FVector2D Position = SentryGameStateUtils::ToNormalizedViewport(World, ScreenPosition, SentryGameStateUtils::GetViewportSize(World));
	const FString Widget = FindWidgetAt(ScreenPosition);

	WriteEvent(Time, TEXT("ui"), TEXT("click"),
		{ { TEXT("widget"), Widget.IsEmpty() ? FString(TEXT("(world)")) : Widget },
			{ TEXT("button"), Button.ToString() },
			{ TEXT("x"), FSentryGameStateJsonWriter::FormatNumber(Position.X, 4) },
			{ TEXT("y"), FSentryGameStateJsonWriter::FormatNumber(Position.Y, 4) } },
		Recorder);
}

const FSentryGameStateSampler::FTrackedActor* FSentryGameStateSampler::FindTrackedActor(AActor* Actor) const
{
	return TrackedActors.FindByPredicate([Actor](const FTrackedActor& Tracked)
	{
		return Tracked.Actor.Get() == Actor;
	});
}

void FSentryGameStateSampler::WriteActorEvent(const FTrackedActor& Tracked, double Time, const TCHAR* Name, FSentryGameStateRecorder& Recorder) const
{
	WriteEvent(Time, TEXT("actor"), Name, { { TEXT("id"), Tracked.Id }, { TEXT("tag"), Tracked.Tag } }, Recorder);
}

void FSentryGameStateSampler::WriteEvent(double Time, const TCHAR* Category, const TCHAR* Name, const TMap<FString, FString>& Data, FSentryGameStateRecorder& Recorder) const
{
	FSentryGameStateJsonWriter Writer;
	Writer.BeginObject()
		.Write(TEXT("type"), TEXT("event"))
		.Write(TEXT("t"), Time)
		.Write(TEXT("category"), Category)
		.Write(TEXT("name"), Name)
		.BeginObject(TEXT("data"));

	for (const TPair<FString, FString>& Pair : Data)
	{
		Writer.Write(*Pair.Key, Pair.Value);
	}

	Writer.EndObject().EndObject();

	Recorder.AddRecord(Time, Writer.GetOutput());
}

void FSentryGameStateSampler::WriteUI(UWorld* World, double Time, FSentryGameStateJsonWriter& Writer, FSentryGameStateRecorder& Recorder)
{
	const FVector2D ViewportSize = SentryGameStateUtils::GetViewportSize(World);
	const FVector2D Mouse = SentryGameStateUtils::ToNormalizedViewport(World, FSlateApplication::Get().GetCursorPos(), ViewportSize);

	Writer.BeginArray(TEXT("viewport"))
		.Write(nullptr, static_cast<double>(ViewportSize.X), 0)
		.Write(nullptr, static_cast<double>(ViewportSize.Y), 0)
		.EndArray();

	Writer.BeginArray(TEXT("mouse"))
		.Write(nullptr, static_cast<double>(Mouse.X), 4)
		.Write(nullptr, static_cast<double>(Mouse.Y), 4)
		.EndArray();

	ViewportWidgets.Reset();

	TSet<FString> CurrentlyOpenWidgets;
	for (TObjectIterator<UUserWidget> It; It; ++It)
	{
		UUserWidget* Widget = *It;
		if (IsValid(Widget) && Widget->GetWorld() == World && Widget->IsInViewport() && Widget->WidgetTree)
		{
			ViewportWidgets.Add(Widget);
			CurrentlyOpenWidgets.Add(Widget->GetClass()->GetName());
		}
	}

	for (const FString& Opened : CurrentlyOpenWidgets.Difference(OpenWidgets))
	{
		WriteEvent(Time, TEXT("ui"), TEXT("open"), { { TEXT("widget"), Opened } }, Recorder);
	}

	for (const FString& Closed : OpenWidgets.Difference(CurrentlyOpenWidgets))
	{
		WriteEvent(Time, TEXT("ui"), TEXT("close"), { { TEXT("widget"), Closed } }, Recorder);
	}

	OpenWidgets = MoveTemp(CurrentlyOpenWidgets);

	if (ViewportWidgets.Num() == 0)
	{
		return;
	}

	Writer.BeginArray(TEXT("ui"));

	int32 NumWidgets = 0;
	for (const TWeakObjectPtr<UUserWidget>& RootWidget : ViewportWidgets)
	{
		const FString RootName = RootWidget->GetClass()->GetName();

		RootWidget->WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			if (!Widget || NumWidgets >= MaxWidgets ||
				!SentryGameStateUtils::IsWidgetVisible(Widget) || !SentryGameStateUtils::HasArea(Widget))
			{
				return;
			}

			const FGeometry& Geometry = Widget->GetCachedGeometry();
			const FVector2D TopLeft = SentryGameStateUtils::ToNormalizedViewport(World, Geometry.LocalToAbsolute(FVector2D::ZeroVector), ViewportSize);
			const FVector2D BottomRight = SentryGameStateUtils::ToNormalizedViewport(World, Geometry.LocalToAbsolute(Geometry.GetLocalSize()), ViewportSize);

			Writer.BeginObject()
				.Write(TEXT("n"), RootName + TEXT("/") + Widget->GetName())
				.Write(TEXT("t"), Widget->GetClass()->GetName())
				.BeginArray(TEXT("r"))
				.Write(nullptr, static_cast<double>(TopLeft.X), 4)
				.Write(nullptr, static_cast<double>(TopLeft.Y), 4)
				.Write(nullptr, static_cast<double>(BottomRight.X - TopLeft.X), 4)
				.Write(nullptr, static_cast<double>(BottomRight.Y - TopLeft.Y), 4)
				.EndArray();

			if (bCaptureUIText)
			{
				if (const UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
				{
					const FString Text = TextBlock->GetText().ToString();
					if (!Text.IsEmpty())
					{
						Writer.Write(TEXT("text"), Text.Left(MaxTextLength));
					}
				}
			}

			if (Widget->IsHovered())
			{
				Writer.Write(TEXT("hover"), true);
			}

			if (!Widget->GetIsEnabled())
			{
				Writer.Write(TEXT("disabled"), true);
			}

			if (const UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
			{
#if UE_VERSION_OLDER_THAN(5, 1, 0)
				const float Percent = ProgressBar->Percent;
#else
				const float Percent = ProgressBar->GetPercent();
#endif
				Writer.Write(TEXT("pct"), static_cast<double>(Percent), 3);
			}

			Writer.EndObject();

			++NumWidgets;
		});
	}

	Writer.EndArray();
}

FString FSentryGameStateSampler::FindWidgetAt(const FVector2D& ScreenPosition) const
{
	FString Found;
	double SmallestArea = TNumericLimits<double>::Max();

	for (const TWeakObjectPtr<UUserWidget>& RootWidget : ViewportWidgets)
	{
		if (!RootWidget.IsValid() || !RootWidget->WidgetTree)
		{
			continue;
		}

		const FString RootName = RootWidget->GetClass()->GetName();

		RootWidget->WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			if (!Widget || !SentryGameStateUtils::IsWidgetVisible(Widget) || Widget->GetVisibility() == ESlateVisibility::HitTestInvisible ||
				!SentryGameStateUtils::HasArea(Widget))
			{
				return;
			}

			const FGeometry& Geometry = Widget->GetCachedGeometry();
			if (!Geometry.IsUnderLocation(ScreenPosition))
			{
				return;
			}

			const FVector2D Size = Geometry.GetLocalSize();
			const double Area = static_cast<double>(Size.X) * Size.Y;
			if (Area < SmallestArea)
			{
				SmallestArea = Area;
				Found = RootName + TEXT("/") + Widget->GetName();
			}
		});
	}

	return Found;
}
