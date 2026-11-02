// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryGameStateIntegration.h"

#include "SentryDefines.h"
#include "SentryModule.h"

#include "GameState/SentryGameStateJsonWriter.h"
#include "GameState/SentryGameStateRecorder.h"
#include "GameState/SentryGameStateSampler.h"
#include "GameState/SentryGameStateUtils.h"

#include "Interface/SentrySubsystemInterface.h"

#include "Async/Async.h"
#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameDelegates.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/Guid.h"

FSentryGameStateIntegration::FSentryGameStateIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

FSentryGameStateIntegration::~FSentryGameStateIntegration() = default;

bool FSentryGameStateIntegration::Setup(const USentrySettings& Settings)
{
	if (!Settings.AttachGameState || IsRunningCommandlet())
	{
		return false;
	}

	FilePath = NativeImpl->GetGameStateFilePath();
	if (FilePath.IsEmpty())
	{
		return false;
	}

	Options = Settings.GameStateOptions;
	Options.WindowSeconds = FMath::Clamp(Options.WindowSeconds, 5, 120);
	Options.SampleInterval = FMath::Clamp(Options.SampleInterval, 0.05f, 1.0f);
	Options.MaxSizeKb = FMath::Clamp(Options.MaxSizeKb, 64, 10240);

	// Crashed runs' game state is already queued with the crash, so leftovers can be safely removed
	DeleteCaptureFiles();

	Recorder = MakeUnique<FSentryGameStateRecorder>(Options.WindowSeconds, Options.MaxSizeKb * 1024);

	Sampler = MakeUnique<FSentryGameStateSampler>();

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FSentryGameStateIntegration::OnTick), Options.SampleInterval);
#else
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("SentryGameStateCapture"), Options.SampleInterval, [this](float DeltaTime)
	{
		return OnTick(DeltaTime);
	});
#endif

	EndPlayMapHandle = FGameDelegates::Get().GetEndPlayMapDelegate().AddRaw(this, &FSentryGameStateIntegration::OnEndPlayMap);

	return true;
}

void FSentryGameStateIntegration::Teardown()
{
	if (TickerHandle.IsValid())
	{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
#else
		FTSTicker::RemoveTicker(TickerHandle);
#endif
		TickerHandle.Reset();
	}

	if (EndPlayMapHandle.IsValid())
	{
		FGameDelegates::Get().GetEndPlayMapDelegate().Remove(EndPlayMapHandle);
		EndPlayMapHandle.Reset();
	}

	if (Recorder)
	{
		EndCapture();
	}

	Sampler.Reset();
	Recorder.Reset();
}

void FSentryGameStateIntegration::AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag)
{
	if (!IsValid(Actor))
	{
		return;
	}

	const FString ActorId = Id.IsEmpty() ? Actor->GetName() : Id;
	const FString ActorTag = Tag.IsEmpty() ? Actor->GetClass()->GetName() : Tag;

	if (Sampler->AddTrackedActor(Actor, ActorId, ActorTag) && bCapturing)
	{
		Sampler->WriteTrackEvent(Actor, GetCaptureTime(), *Recorder);
	}
}

void FSentryGameStateIntegration::RemoveTrackedActor(AActor* Actor)
{
	if (bCapturing)
	{
		Sampler->WriteUntrackEvent(Actor, GetCaptureTime(), *Recorder);
	}

	Sampler->RemoveTrackedActor(Actor);
}

void FSentryGameStateIntegration::SetAttribute(const FString& Key, const FSentryVariant& Value)
{
	Sampler->SetAttribute(Key, Value);
}

void FSentryGameStateIntegration::RemoveAttribute(const FString& Key)
{
	Sampler->RemoveAttribute(Key);
}

bool FSentryGameStateIntegration::OnTick(float DeltaTime)
{
	UWorld* World = FindGameWorld();
	if (!World)
	{
		return true;
	}

	if (!bCapturing)
	{
		BeginCapture(World);
	}
	else if (World != CapturedWorld.Get())
	{
		OnWorldChanged(World);
	}

	Sampler->SampleFrame(World, GetCaptureTime(), *Recorder);

	const double Now = FPlatformTime::Seconds();
	if (Now - LastFlushSeconds >= FlushIntervalSeconds)
	{
		LastFlushSeconds = Now;
		FlushToFile();
	}

	return true;
}

void FSentryGameStateIntegration::OnWorldChanged(UWorld* World)
{
	CapturedWorld = World;
	UpdateHeader(World);

	Sampler->WriteMapChangedEvent(World, GetCaptureTime(), *Recorder);
}

void FSentryGameStateIntegration::OnEndPlayMap()
{
	// PIE session ended - drop its game state so that later editor crashes don't report it
	EndCapture();
}

void FSentryGameStateIntegration::BeginCapture(UWorld* World)
{
	CaptureId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens).ToLower();
	CaptureStartSeconds = FPlatformTime::Seconds();
	CaptureStartUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
	LastFlushSeconds = CaptureStartSeconds;

	CapturedWorld = World;
	bCapturing = true;

	Recorder->Reset();

	UpdateHeader(World);

	Sampler->WriteInitialTrackEvents(GetCaptureTime(), *Recorder);

	UE_LOG(LogSentrySdk, Log, TEXT("Game state capture started (%.2f s interval, %d s window)."), Options.SampleInterval, Options.WindowSeconds);
}

void FSentryGameStateIntegration::EndCapture()
{
	if (!bCapturing)
	{
		return;
	}

	bCapturing = false;
	CapturedWorld.Reset();

	Recorder->Reset();
	Sampler->Reset();

	WaitForPendingWrite();
	DeleteCaptureFiles();
}

void FSentryGameStateIntegration::UpdateHeader(UWorld* World)
{
	FSentryGameStateJsonWriter Writer;
	Writer.BeginObject()
		.Write(TEXT("type"), TEXT("header"))
		.Write(TEXT("schema"), 1)
		.Write(TEXT("sdk"), TEXT("sentry.unreal"))
		.Write(TEXT("sdkVersion"), FSentryModule::GetPluginVersion())
		.Write(TEXT("captureId"), CaptureId)
		.Write(TEXT("startedAtUnix"), CaptureStartUnixTime)
		.Write(TEXT("startedAtIso"), FDateTime::FromUnixTimestamp(CaptureStartUnixTime).ToIso8601())
		.Write(TEXT("game"), FApp::GetProjectName())
		.Write(TEXT("map"), UWorld::RemovePIEPrefix(World->GetMapName()))
		.Write(TEXT("engine"), FEngineVersion::Current().ToString())
		.Write(TEXT("netMode"), SentryGameStateUtils::NetModeToString(World->GetNetMode()));

	if (World->WorldType == EWorldType::PIE && GEngine)
	{
		if (const FWorldContext* Context = GEngine->GetWorldContextFromWorld(World))
		{
			Writer.Write(TEXT("pieInstance"), Context->PIEInstance);
		}
	}

	Writer.Write(TEXT("sampleInterval"), static_cast<double>(Options.SampleInterval))
		.Write(TEXT("windowSeconds"), Options.WindowSeconds)
		.Write(TEXT("timeUnit"), TEXT("seconds_since_capture_start"))
		.Write(TEXT("units"), TEXT("cm"))
		.Write(TEXT("coordinateSystem"), TEXT("unreal-left-handed-z-up"))
		.Write(TEXT("rotationOrder"), TEXT("pitch_yaw_roll_deg"))
		.EndObject();

	Recorder->SetHeader(Writer.GetOutput());
}

void FSentryGameStateIntegration::FlushToFile()
{
	if (PendingWrite.IsValid() && !PendingWrite.IsReady())
	{
		return;
	}

	PendingWrite = Async(EAsyncExecution::ThreadPool, [Path = FilePath, Data = Recorder->BuildSnapshot()]()
	{
		SentryGameStateUtils::WriteSnapshotFile(Path, Data);
	});
}

void FSentryGameStateIntegration::WaitForPendingWrite()
{
	if (PendingWrite.IsValid())
	{
		PendingWrite.Wait();
		PendingWrite.Reset();
	}
}

void FSentryGameStateIntegration::DeleteCaptureFiles()
{
	IFileManager::Get().Delete(*FilePath, false, true, true);
	IFileManager::Get().Delete(*(FilePath + TEXT(".tmp")), false, true, true);
}

double FSentryGameStateIntegration::GetCaptureTime() const
{
	return FPlatformTime::Seconds() - CaptureStartSeconds;
}

UWorld* FSentryGameStateIntegration::FindGameWorld() const
{
	if (!GEngine)
	{
		return nullptr;
	}

	UWorld* BestWorld = nullptr;
	int32 BestScore = MIN_int32;

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (!World)
		{
			continue;
		}

		if (Context.WorldType == EWorldType::Game)
		{
			return World;
		}

		if (Context.WorldType != EWorldType::PIE)
		{
			continue;
		}

		// Prefer the PIE instance with a local player, then the lowest instance number
		const int32 Score = (Context.GameViewport != nullptr ? 1000 : 0) - Context.PIEInstance;
		if (Score > BestScore)
		{
			BestScore = Score;
			BestWorld = World;
		}
	}

	return BestWorld;
}
