// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryGameStateTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryVariant.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Guid.h"

namespace
{
constexpr double DestroyActorAfterSeconds = 3.0;
constexpr double CrashAfterSeconds = 7.5;
constexpr float OrbitRadius = 500.0f;

const TCHAR* const ActorTags[] = { TEXT("enemy"), TEXT("enemy"), TEXT("pickup") };
} // namespace

void FSentryGameStateTest::Run()
{
	if (!FSentryModule::Get().GetSettings()->AttachGameState)
	{
		CompleteWithResult(false, TEXT("Game state capture is disabled"));
		return;
	}

	// Actors can only be spawned once the world begins play, so drive the test from the frame loop.
	// Capture a shared reference so this test stays alive past RunIntegrationTest returning.
	TSharedRef<FSentryGameStateTest> Self = StaticCastSharedRef<FSentryGameStateTest>(AsShared());

	FCoreDelegates::OnEndFrame.AddLambda([Self]()
	{
		Self->Tick();
	});
}

void FSentryGameStateTest::Tick()
{
	UWorld* World = GWorld;
	if (bCrashed || !World || !World->HasBegunPlay())
	{
		return;
	}

	if (!bSpawned)
	{
		SpawnActors(World);
		StartTime = FPlatformTime::Seconds();
		bSpawned = true;
	}

	USentrySubsystem* Subsystem = GetSubsystem();

	const double Elapsed = FPlatformTime::Seconds() - StartTime;

	for (int32 Index = 0; Index < Actors.Num(); ++Index)
	{
		if (AActor* Actor = Actors[Index].Get())
		{
			const float Angle = static_cast<float>(Elapsed) * (1.0f + Index * 0.5f) + Index * 2.0f;
			Actor->SetActorLocationAndRotation(
				FVector(FMath::Cos(Angle) * OrbitRadius, FMath::Sin(Angle) * OrbitRadius, 100.0f + Index * 50.0f),
				FRotator(0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
		}
	}

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			Pawn->SetActorLocation(FVector(static_cast<float>(Elapsed) * 100.0f, 0.0f, 200.0f), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	Subsystem->SetGameStateAttribute(TEXT("score"), FSentryVariant(static_cast<int32>(Elapsed * 10.0)));
	Subsystem->SetGameStateAttribute(TEXT("wave"), FSentryVariant(1 + static_cast<int32>(Elapsed / 2.0)));

	if (!bDestroyedActor && Elapsed >= DestroyActorAfterSeconds)
	{
		if (AActor* Actor = Actors.Last().Get())
		{
			Actor->Destroy();
		}

		Subsystem->SetGameStateAttribute(TEXT("phase"), FSentryVariant(TEXT("combat")));
		bDestroyedActor = true;
	}

	if (Elapsed >= CrashAfterSeconds)
	{
		Crash();
	}
}

void FSentryGameStateTest::SpawnActors(UWorld* World)
{
	USentrySubsystem* Subsystem = GetSubsystem();

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ActorTags); ++Index)
	{
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (!Actor)
		{
			continue;
		}

		USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
		Root->SetMobility(EComponentMobility::Movable);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();

		Subsystem->AddGameStateTrackedActor(Actor, FString::Printf(TEXT("Orbiter_%d"), Index), ActorTags[Index]);
		Actors.Add(Actor);
	}

	Subsystem->SetGameStateAttribute(TEXT("phase"), FSentryVariant(TEXT("warmup")));
}

void FSentryGameStateTest::Crash()
{
	bCrashed = true;

	// Because we don't get the real crash event ID, create a fake one and set it as a tag
	// This tag is then used by integration test script in CI to fetch the event
	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Flush logs to ensure output is captured before crash
	GLog->Flush();

	GetSubsystem()->SetTag(TEXT("test.crash_id"), EventId);

	USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType::NullPointer);
}
