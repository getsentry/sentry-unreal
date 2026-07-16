// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryStateReplaySubsystem.h"

#include "SentryStateReplaySettings.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Containers/StringConv.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Input/Events.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY_STATIC(LogSentryStateReplay, Log, All);

/**
 * Slate-level input pre-processor. Sees every pointer press before UMG routes
 * it, so clicks on Blueprint menu buttons are captured even while gameplay is
 * paused and the PlayerController receives no input. Never consumes events.
 */
class FSentryStateReplayInputProcessor : public IInputProcessor
{
public:
	explicit FSentryStateReplayInputProcessor(USentryStateReplaySubsystem* InOwner)
		: Owner(InOwner)
	{
	}

	virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication&, const FPointerEvent& MouseEvent) override
	{
		if (USentryStateReplaySubsystem* Sub = Owner.Get())
		{
			Sub->OnUIClick(MouseEvent.GetScreenSpacePosition(), MouseEvent.GetEffectingButton().ToString());
		}
		return false;
	}

	virtual const TCHAR* GetDebugName() const override { return TEXT("SentryStateReplay"); }

private:
	TWeakObjectPtr<USentryStateReplaySubsystem> Owner;
};

namespace
{
FString EscapeJson(const FString& In)
{
	FString Out;
	Out.Reserve(In.Len() + 8);
	for (const TCHAR Ch : In)
	{
		switch (Ch)
		{
		case '\"':
			Out += TEXT("\\\"");
			break;
		case '\\':
			Out += TEXT("\\\\");
			break;
		case '\n':
			Out += TEXT("\\n");
			break;
		case '\r':
			Out += TEXT("\\r");
			break;
		case '\t':
			Out += TEXT("\\t");
			break;
		default:
			if (Ch < 0x20)
			{
				Out += FString::Printf(TEXT("\\u%04x"), static_cast<int32>(Ch));
			}
			else
			{
				Out.AppendChar(Ch);
			}
		}
	}
	return Out;
}

FString Vec3(const FVector& V)
{
	return FString::Printf(TEXT("[%.2f,%.2f,%.2f]"), V.X, V.Y, V.Z);
}

FString Rot3(const FRotator& R)
{
	return FString::Printf(TEXT("[%.2f,%.2f,%.2f]"), R.Pitch, R.Yaw, R.Roll);
}

FString JsonObjFromMap(const TMap<FString, FString>& Data)
{
	FString Out = TEXT("{");
	bool bFirst = true;
	for (const TPair<FString, FString>& Pair : Data)
	{
		if (!bFirst)
		{
			Out += TEXT(",");
		}
		bFirst = false;
		Out += FString::Printf(TEXT("\"%s\":\"%s\""), *EscapeJson(Pair.Key), *EscapeJson(Pair.Value));
	}
	Out += TEXT("}");
	return Out;
}

// Extracts displayed text for the common text-bearing widget types.
FString WidgetText(const UWidget* Widget)
{
	if (const UTextBlock* Tb = Cast<UTextBlock>(Widget))
	{
		return Tb->GetText().ToString();
	}
	if (const UEditableTextBox* Etb = Cast<UEditableTextBox>(Widget))
	{
		return Etb->GetText().ToString();
	}
	if (const UEditableText* Et = Cast<UEditableText>(Widget))
	{
		return Et->GetText().ToString();
	}
	return FString();
}

bool IsWidgetVisible(ESlateVisibility Vis)
{
	return Vis != ESlateVisibility::Collapsed && Vis != ESlateVisibility::Hidden;
}
} // namespace

USentryStateReplaySubsystem* USentryStateReplaySubsystem::Get(const UObject* WorldContextObject)
{
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		return GameInstance->GetSubsystem<USentryStateReplaySubsystem>();
	}
	return nullptr;
}

void USentryStateReplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const USentryStateReplaySettings* Settings = GetDefault<USentryStateReplaySettings>();
	bEnabled = Settings->bEnabled;
	bAutoStart = Settings->bAutoStart;
	bCaptureUI = Settings->bCaptureUI;
	SampleRateHz = FMath::Clamp(Settings->SampleRateHz, 1, 120);
	OutputSubdir = Settings->OutputSubdir.IsEmpty() ? TEXT("StateReplays") : Settings->OutputSubdir;
	FlushIntervalSeconds = FMath::Max(0.1f, Settings->FlushIntervalSeconds);

	if (!bEnabled)
	{
		return;
	}

	const float Period = 1.0f / static_cast<float>(SampleRateHz);
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &USentryStateReplaySubsystem::Tick), Period);

	UE_LOG(LogSentryStateReplay, Log, TEXT("State replay enabled (%d Hz, auto-start=%d)"), SampleRateHz, bAutoStart ? 1 : 0);
}

void USentryStateReplaySubsystem::Deinitialize()
{
	StopSession();

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	Super::Deinitialize();
}

UWorld* USentryStateReplaySubsystem::ResolveGameWorld() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

double USentryStateReplaySubsystem::Now() const
{
	return FPlatformTime::Seconds() - SessionStartSeconds;
}

bool USentryStateReplaySubsystem::Tick(float DeltaTime)
{
	UWorld* World = ResolveGameWorld();
	if (!World)
	{
		return true;
	}

	if (!bSessionActive)
	{
		if (bAutoStart && World->GetFirstPlayerController() != nullptr)
		{
			StartSession(TEXT("auto"));
		}
		return true;
	}

	const FString MapName = World->RemovePIEPrefix(World->GetMapName());
	if (MapName != CurrentMapName)
	{
		CurrentMapName = MapName;
		AddEvent(TEXT("world"), TEXT("map_changed"), { { TEXT("map"), MapName } });
	}

	SampleFrame(World, Now());

	const double NowSeconds = FPlatformTime::Seconds();
	if (NowSeconds - LastFlushSeconds >= FlushIntervalSeconds)
	{
		FScopeLock Lock(&WriterLock);
		if (FileWriter)
		{
			FileWriter->Flush();
		}
		LastFlushSeconds = NowSeconds;
	}

	return true;
}

void USentryStateReplaySubsystem::SampleFrame(UWorld* World, double SessionTime)
{
	FString Frame = FString::Printf(TEXT("{\"type\":\"frame\",\"t\":%.3f"), SessionTime);

	const float FrameDelta = FApp::GetDeltaTime();
	Frame += FString::Printf(TEXT(",\"perf\":{\"fps\":%.1f,\"ms\":%.2f}"),
		FrameDelta > 0.f ? 1.f / FrameDelta : 0.f, FrameDelta * 1000.f);

	if (const APlayerController* PC = World->GetFirstPlayerController())
	{
		if (const APawn* Pawn = PC->GetPawn())
		{
			Frame += FString::Printf(TEXT(",\"player\":{\"loc\":%s,\"rot\":%s}"),
				*Vec3(Pawn->GetActorLocation()), *Rot3(Pawn->GetActorRotation()));
		}

		if (const APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			Frame += FString::Printf(TEXT(",\"camera\":{\"loc\":%s,\"rot\":%s,\"fov\":%.1f}"),
				*Vec3(Cam->GetCameraLocation()), *Rot3(Cam->GetCameraRotation()), Cam->GetFOVAngle());
		}
	}

	// Tracked actors: prune dead weak refs (emitting a despawn) and serialize the rest.
	int32 AliveEntities = 0;
	TMap<FString, int32> TagCounts;
	{
		FString Actors;
		bool bFirst = true;
		for (int32 Index = TrackedActors.Num() - 1; Index >= 0; --Index)
		{
			const FTrackedActor& Tracked = TrackedActors[Index];
			const AActor* Actor = Tracked.Actor.Get();
			if (!Actor)
			{
				AddEvent(TEXT("actor"), TEXT("despawn"), { { TEXT("id"), Tracked.Id }, { TEXT("tag"), Tracked.Tag } });
				TrackedActors.RemoveAt(Index);
				continue;
			}

			++AliveEntities;
			TagCounts.FindOrAdd(Tracked.Tag)++;

			if (!bFirst)
			{
				Actors += TEXT(",");
			}
			bFirst = false;
			Actors += FString::Printf(TEXT("{\"id\":\"%s\",\"tag\":\"%s\",\"loc\":%s,\"rot\":%s}"),
				*EscapeJson(Tracked.Id), *EscapeJson(Tracked.Tag),
				*Vec3(Actor->GetActorLocation()), *Rot3(Actor->GetActorRotation()));
		}
		if (!Actors.IsEmpty())
		{
			Frame += FString::Printf(TEXT(",\"actors\":[%s]"), *Actors);
		}
	}

	// Game-state channel: auto entity counts + any user-provided SetState values.
	{
		FString StateJson;
		bool bFirst = true;
		auto AddNum = [&StateJson, &bFirst](const FString& Key, double Value)
		{
			StateJson += FString::Printf(TEXT("%s\"%s\":%g"), bFirst ? TEXT("") : TEXT(","), *EscapeJson(Key), Value);
			bFirst = false;
		};
		AddNum(TEXT("entities"), AliveEntities);
		for (const TPair<FString, int32>& KV : TagCounts)
		{
			AddNum(KV.Key, KV.Value);
		}
		for (const TPair<FString, float>& KV : NumericState)
		{
			AddNum(KV.Key, KV.Value);
		}
		for (const TPair<FString, FString>& KV : TextState)
		{
			StateJson += FString::Printf(TEXT("%s\"%s\":\"%s\""), bFirst ? TEXT("") : TEXT(","), *EscapeJson(KV.Key), *EscapeJson(KV.Value));
			bFirst = false;
		}
		Frame += FString::Printf(TEXT(",\"state\":{%s}"), *StateJson);
	}

	if (bCaptureUI && FSlateApplication::IsInitialized())
	{
		const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(World);
		Frame += FString::Printf(TEXT(",\"viewport\":[%.0f,%.0f]"), ViewportSize.X, ViewportSize.Y);

		const double VW = FMath::Max(1.0, ViewportSize.X);
		const double VH = FMath::Max(1.0, ViewportSize.Y);

		FVector2D CursorPx, Unused;
		USlateBlueprintLibrary::AbsoluteToViewport(World, FSlateApplication::Get().GetCursorPos(), CursorPx, Unused);
		Frame += FString::Printf(TEXT(",\"mouse\":[%.4f,%.4f]"), CursorPx.X / VW, CursorPx.Y / VH);

		// Detect top-level widget open/close for the event timeline.
		TSet<FString> Tops;
		for (TObjectIterator<UUserWidget> It; It; ++It)
		{
			const UUserWidget* Widget = *It;
			if (IsValid(Widget) && Widget->GetWorld() == World && Widget->IsInViewport())
			{
				Tops.Add(Widget->GetClass()->GetName());
			}
		}
		for (const FString& W : Tops)
		{
			if (!ActiveTopWidgets.Contains(W))
			{
				AddEvent(TEXT("ui"), TEXT("open"), { { TEXT("widget"), W } });
			}
		}
		for (const FString& W : ActiveTopWidgets)
		{
			if (!Tops.Contains(W))
			{
				AddEvent(TEXT("ui"), TEXT("close"), { { TEXT("widget"), W } });
			}
		}
		ActiveTopWidgets = MoveTemp(Tops);

		FString UiArray;
		CaptureUI(World, UiArray);
		if (!UiArray.IsEmpty())
		{
			Frame += FString::Printf(TEXT(",\"ui\":[%s]"), *UiArray);
		}
	}

	Frame += TEXT("}");
	WriteLine(Frame);
}

void USentryStateReplaySubsystem::CaptureUI(UWorld* World, FString& Out)
{
	Out.Reset();

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(World);
	const double VW = FMath::Max(1.0, ViewportSize.X);
	const double VH = FMath::Max(1.0, ViewportSize.Y);

	constexpr int32 MaxWidgets = 256;
	int32 Count = 0;
	bool bFirst = true;

	for (TObjectIterator<UUserWidget> It; It; ++It)
	{
		UUserWidget* Root = *It;
		if (!IsValid(Root) || Root->GetWorld() != World || !Root->IsInViewport() || !Root->WidgetTree)
		{
			continue;
		}

		const FString RootName = Root->GetClass()->GetName();
		Root->WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			if (!Widget || Count >= MaxWidgets || !IsWidgetVisible(Widget->GetVisibility()))
			{
				return;
			}

			const FGeometry& Geometry = Widget->GetCachedGeometry();
			const FVector2D LocalSize = Geometry.GetLocalSize();
			if (LocalSize.X < 1.0 || LocalSize.Y < 1.0)
			{
				return;
			}

			FVector2D TopLeftPx, BottomRightPx, Unused;
			USlateBlueprintLibrary::AbsoluteToViewport(World, Geometry.LocalToAbsolute(FVector2D::ZeroVector), TopLeftPx, Unused);
			USlateBlueprintLibrary::AbsoluteToViewport(World, Geometry.LocalToAbsolute(LocalSize), BottomRightPx, Unused);

			const double NX = TopLeftPx.X / VW;
			const double NY = TopLeftPx.Y / VH;
			const double NW = (BottomRightPx.X - TopLeftPx.X) / VW;
			const double NH = (BottomRightPx.Y - TopLeftPx.Y) / VH;

			if (!bFirst)
			{
				Out += TEXT(",");
			}
			bFirst = false;

			Out += FString::Printf(TEXT("{\"n\":\"%s/%s\",\"t\":\"%s\",\"r\":[%.4f,%.4f,%.4f,%.4f]"),
				*EscapeJson(RootName), *EscapeJson(Widget->GetName()), *EscapeJson(Widget->GetClass()->GetName()),
				NX, NY, NW, NH);

			const FString Text = WidgetText(Widget);
			if (!Text.IsEmpty())
			{
				Out += FString::Printf(TEXT(",\"text\":\"%s\""), *EscapeJson(Text.Left(120)));
			}
			if (Widget->IsHovered())
			{
				Out += TEXT(",\"hover\":1");
			}
			if (!Widget->GetIsEnabled())
			{
				Out += TEXT(",\"disabled\":1");
			}
			if (const UProgressBar* ProgressBar = Cast<UProgressBar>(Widget))
			{
				Out += FString::Printf(TEXT(",\"pct\":%.3f"), ProgressBar->GetPercent());
			}
			Out += TEXT("}");

			++Count;
		});
	}
}

FString USentryStateReplaySubsystem::ResolveWidgetAt(const FVector2D& ScreenPos, UWorld* World) const
{
	FString Best;
	double BestArea = TNumericLimits<double>::Max();

	for (TObjectIterator<UUserWidget> It; It; ++It)
	{
		UUserWidget* Root = *It;
		if (!IsValid(Root) || Root->GetWorld() != World || !Root->IsInViewport() || !Root->WidgetTree)
		{
			continue;
		}

		const FString RootName = Root->GetClass()->GetName();
		Root->WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			if (!Widget)
			{
				return;
			}
			const ESlateVisibility Vis = Widget->GetVisibility();
			if (!IsWidgetVisible(Vis) || Vis == ESlateVisibility::HitTestInvisible)
			{
				return;
			}
			const FGeometry& Geometry = Widget->GetCachedGeometry();
			const FVector2D LocalSize = Geometry.GetLocalSize();
			if (LocalSize.X < 1.0 || LocalSize.Y < 1.0)
			{
				return;
			}
			if (Geometry.IsUnderLocation(ScreenPos))
			{
				const double Area = LocalSize.X * LocalSize.Y;
				if (Area < BestArea)
				{
					BestArea = Area;
					Best = RootName + TEXT("/") + Widget->GetName();
				}
			}
		});
	}

	return Best;
}

void USentryStateReplaySubsystem::OnUIClick(const FVector2D& ScreenPos, const FString& Button)
{
	if (!bSessionActive)
	{
		return;
	}
	UWorld* World = ResolveGameWorld();
	if (!World)
	{
		return;
	}

	const FString Widget = ResolveWidgetAt(ScreenPos, World);

	FVector2D Px, Unused;
	USlateBlueprintLibrary::AbsoluteToViewport(World, ScreenPos, Px, Unused);
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(World);
	const float VW = FMath::Max(1.0f, static_cast<float>(ViewportSize.X));
	const float VH = FMath::Max(1.0f, static_cast<float>(ViewportSize.Y));

	AddEvent(TEXT("ui"), TEXT("click"), {
											{ TEXT("widget"), Widget.IsEmpty() ? TEXT("(world)") : Widget },
											{ TEXT("button"), Button },
											{ TEXT("x"), FString::Printf(TEXT("%.4f"), Px.X / VW) },
											{ TEXT("y"), FString::Printf(TEXT("%.4f"), Px.Y / VH) },
										});
}

void USentryStateReplaySubsystem::StartSession(const FString& Label)
{
	if (bSessionActive)
	{
		return;
	}

	const FDateTime NowUtc = FDateTime::UtcNow();
	const FString Stamp = NowUtc.ToString(TEXT("%Y%m%d-%H%M%S"));
	const FString SafeLabel = Label.IsEmpty() ? TEXT("session") : Label;
	const FString FileName = FString::Printf(TEXT("session-%s-%s.jsonl"), *Stamp, *SafeLabel);

	SessionFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), OutputSubdir, FileName);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(SessionFilePath), true);

	{
		FScopeLock Lock(&WriterLock);
		FileWriter.Reset(IFileManager::Get().CreateFileWriter(*SessionFilePath, FILEWRITE_EvenIfReadOnly));
	}
	if (!FileWriter)
	{
		UE_LOG(LogSentryStateReplay, Warning, TEXT("State replay: failed to open %s"), *SessionFilePath);
		return;
	}

	SessionStartSeconds = FPlatformTime::Seconds();
	LastFlushSeconds = SessionStartSeconds;
	bSessionActive = true;
	ActiveTopWidgets.Reset();

	UWorld* World = ResolveGameWorld();
	CurrentMapName = World ? World->RemovePIEPrefix(World->GetMapName()) : FString();

	const FString Header = FString::Printf(
		TEXT("{\"type\":\"header\",\"schema\":2,\"sdk\":\"sentry-unreal-state-replay\",\"version\":\"0.2.0\",")
			TEXT("\"session\":\"%s\",\"startedAtUnix\":%lld,\"startedAtIso\":\"%s\",\"game\":\"%s\",\"map\":\"%s\",")
				TEXT("\"engine\":\"%s\",\"sampleRateHz\":%d,\"units\":\"cm\",\"coordinateSystem\":\"unreal-left-handed-z-up\",")
					TEXT("\"rotationOrder\":\"pitch_yaw_roll_deg\",\"uiCoords\":\"viewport-normalized-0-1\"}"),
		*EscapeJson(SafeLabel), NowUtc.ToUnixTimestamp(), *NowUtc.ToIso8601(),
		*EscapeJson(FApp::GetProjectName()), *EscapeJson(CurrentMapName),
		*EscapeJson(FEngineVersion::Current().ToString()), SampleRateHz);

	{
		FScopeLock Lock(&WriterLock);
		WriteLineUnlocked(Header);
	}

	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FSentryStateReplayInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}

	UE_LOG(LogSentryStateReplay, Log, TEXT("State replay session started: %s"), *SessionFilePath);
}

void USentryStateReplaySubsystem::StopSession()
{
	if (!bSessionActive)
	{
		return;
	}
	bSessionActive = false;

	if (InputProcessor.IsValid())
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		}
		InputProcessor.Reset();
	}

	FScopeLock Lock(&WriterLock);
	if (FileWriter)
	{
		FileWriter->Flush();
		FileWriter->Close();
		FileWriter.Reset();
	}
	TrackedActors.Reset();
	NumericState.Reset();
	TextState.Reset();

	UE_LOG(LogSentryStateReplay, Log, TEXT("State replay session stopped: %s"), *SessionFilePath);
}

void USentryStateReplaySubsystem::AddEvent(const FString& Category, const FString& Name, const TMap<FString, FString>& Data)
{
	if (!bSessionActive)
	{
		return;
	}

	const FString Line = FString::Printf(
		TEXT("{\"type\":\"event\",\"t\":%.3f,\"category\":\"%s\",\"name\":\"%s\",\"data\":%s}"),
		Now(), *EscapeJson(Category), *EscapeJson(Name), *JsonObjFromMap(Data));
	WriteLine(Line);
}

void USentryStateReplaySubsystem::AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag)
{
	if (!IsValid(Actor))
	{
		return;
	}
	for (const FTrackedActor& Existing : TrackedActors)
	{
		if (Existing.Actor.Get() == Actor)
		{
			return;
		}
	}
	TrackedActors.Add(FTrackedActor{ Actor, Id, Tag });
	AddEvent(TEXT("actor"), TEXT("spawn"), { { TEXT("id"), Id }, { TEXT("tag"), Tag } });
}

void USentryStateReplaySubsystem::RemoveTrackedActor(AActor* Actor)
{
	for (int32 Index = 0; Index < TrackedActors.Num(); ++Index)
	{
		if (TrackedActors[Index].Actor.Get() == Actor)
		{
			TrackedActors.RemoveAt(Index);
			return;
		}
	}
}

void USentryStateReplaySubsystem::SetState(const FString& Key, float Value)
{
	NumericState.Add(Key, Value);
}

void USentryStateReplaySubsystem::SetStateString(const FString& Key, const FString& Value)
{
	TextState.Add(Key, Value);
}

void USentryStateReplaySubsystem::WriteLine(const FString& Line)
{
	FScopeLock Lock(&WriterLock);
	WriteLineUnlocked(Line);
}

void USentryStateReplaySubsystem::WriteLineUnlocked(const FString& Line)
{
	if (!FileWriter)
	{
		return;
	}
	FTCHARToUTF8 Utf8(*Line);
	FileWriter->Serialize(const_cast<ANSICHAR*>(reinterpret_cast<const ANSICHAR*>(Utf8.Get())), Utf8.Length());
	uint8 Newline = '\n';
	FileWriter->Serialize(&Newline, 1);
}
