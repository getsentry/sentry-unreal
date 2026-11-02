// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateUtils.h"

#include "SentryDefines.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Widget.h"
#include "Containers/StringConv.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"

TArray<uint8> SentryGameStateUtils::ToUtf8Line(const FString& Line)
{
	FTCHARToUTF8 Converted(*Line);

	TArray<uint8> Data;
	Data.Reserve(Converted.Length() + 1);
	Data.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	Data.Add('\n');
	return Data;
}

const TCHAR* SentryGameStateUtils::NetModeToString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_DedicatedServer:
		return TEXT("dedicated_server");
	case NM_ListenServer:
		return TEXT("listen_server");
	case NM_Client:
		return TEXT("client");
	default:
		return TEXT("standalone");
	}
}

void SentryGameStateUtils::WriteSnapshotFile(const FString& Path, const TArray<uint8>& Data)
{
	const FString TempPath = Path + TEXT(".tmp");

	TUniquePtr<FArchive> Writer(IFileManager::Get().CreateFileWriter(*TempPath, FILEWRITE_EvenIfReadOnly));
	if (!Writer)
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to open %s for write"), *TempPath);
		return;
	}

	Writer->Serialize(const_cast<uint8*>(Data.GetData()), Data.Num());

	if (!Writer->Close())
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to write %s"), *TempPath);
		return;
	}

	if (!IFileManager::Get().Move(*Path, *TempPath, true, true, false, true))
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to replace %s"), *Path);
	}
}

bool SentryGameStateUtils::IsWidgetVisible(const UWidget* Widget)
{
	const ESlateVisibility Visibility = Widget->GetVisibility();
	return Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden;
}

bool SentryGameStateUtils::HasArea(const UWidget* Widget)
{
	const FVector2D Size = Widget->GetCachedGeometry().GetLocalSize();
	return Size.X >= 1.0f && Size.Y >= 1.0f;
}

FVector2D SentryGameStateUtils::GetViewportSize(UWorld* World)
{
	const FVector2D Size = UWidgetLayoutLibrary::GetViewportSize(World);
	return FVector2D(FMath::Max(1.0f, static_cast<float>(Size.X)), FMath::Max(1.0f, static_cast<float>(Size.Y)));
}

FVector2D SentryGameStateUtils::ToNormalizedViewport(UWorld* World, const FVector2D& AbsolutePosition, const FVector2D& ViewportSize)
{
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::AbsoluteToViewport(World, AbsolutePosition, PixelPosition, ViewportPosition);
	return PixelPosition / ViewportSize;
}
