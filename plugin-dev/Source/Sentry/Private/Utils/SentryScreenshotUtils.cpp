// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryScreenshotUtils.h"

#include "HighResScreenshot.h"
#include "SentryDefines.h"

#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "UnrealClient.h"
#include "Misc/FileHelper.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/EngineVersionComparison.h"

bool SentryScreenshotUtils::CaptureScreenshot(const FString& ScreenshotSavePath)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("GameViewport required for screenshot capturing is not valid"));
		return false;
	}

	UGameViewportClient* GameViewportClient = GEngine->GameViewport;
	if (!GameViewportClient)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Game Viewport Client required for screenshot capturing is not valid"));
		return false;
	}

	FIntVector ViewportSize(GameViewportClient->Viewport->GetSizeXY().X, GameViewportClient->Viewport->GetSizeXY().Y, 0);

	TArray<FColor> Bitmap;

	if (!FSlateApplication::IsInitialized())
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Slate application required for screenshot capturing is not initialized"));
		return false;
	}

	TSharedPtr<SWindow> WindowPtr = GameViewportClient->GetWindow();
	TSharedRef<SWidget> WindowRef = WindowPtr.ToSharedRef();

	bool bScreenshotSuccessful = FSlateApplication::Get().TakeScreenshot(WindowRef, Bitmap, ViewportSize);
	if (!bScreenshotSuccessful)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to capture screenshot"));
		return false;
	}

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	GetHighResScreenshotConfig().MergeMaskIntoAlpha(Bitmap);
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(ViewportSize.X, ViewportSize.Y, Bitmap, CompressedBitmap);
#else
	GetHighResScreenshotConfig().MergeMaskIntoAlpha(Bitmap, FIntRect());
	TArray64<uint8> CompressedBitmap;
	FImageUtils::PNGCompressImageArray(ViewportSize.X, ViewportSize.Y, Bitmap, CompressedBitmap);
#endif

	FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenshotSavePath);

	UE_LOG(LogSentrySdk, Log, TEXT("Screenshot saved to: %s"), *ScreenshotSavePath);

	return true;
}
