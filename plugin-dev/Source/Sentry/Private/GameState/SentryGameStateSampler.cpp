// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateSampler.h"

#include "GameState/SentryGameStateJsonWriter.h"
#include "GameState/SentryGameStateRecorder.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/App.h"

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

void FSentryGameStateSampler::Reset()
{
	TrackedActors.Reset();
	Attributes.Reset();
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
