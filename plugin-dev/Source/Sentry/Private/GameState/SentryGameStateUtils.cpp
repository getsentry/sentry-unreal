// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Widget.h"
#include "Containers/StringConv.h"
#include "Engine/World.h"

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
