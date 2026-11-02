// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateSampler.h"

#include "GameState/SentryGameStateJsonWriter.h"
#include "GameState/SentryGameStateRecorder.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
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

	Writer.EndObject();

	Recorder.AddRecord(Time, Writer.GetOutput());
}

void FSentryGameStateSampler::Reset()
{
}

void FSentryGameStateSampler::WriteMapChangedEvent(UWorld* World, double Time, FSentryGameStateRecorder& Recorder) const
{
	WriteEvent(Time, TEXT("world"), TEXT("map_changed"), { { TEXT("map"), UWorld::RemovePIEPrefix(World->GetMapName()) } }, Recorder);
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
