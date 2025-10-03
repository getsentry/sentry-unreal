// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryScreenshotUtils.h"
#include "SentryDefines.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

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

	TArray<FColor>* Bitmap = new TArray<FColor>();
	Bitmap->SetNumZeroed(ViewportSize.X * ViewportSize.Y);

	if (!FSlateApplication::IsInitialized())
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Slate application required for screenshot capturing is not initialized"));
		return false;
	}

	if (!(IsInGameThread() || IsInSlateThread()))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Can't take a screenshot when not in the game thread or slate thread"));
		return false;
	}

	TSharedPtr<SWindow> WindowPtr = GameViewportClient->GetWindow();
	TSharedRef<SWidget> WindowRef = WindowPtr.ToSharedRef();

	bool bScreenshotSuccessful = FSlateApplication::Get().TakeScreenshot(WindowRef, *Bitmap, ViewportSize);
	if (!bScreenshotSuccessful)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to capture screenshot"));
		return false;
	}

#if PLATFORM_ANDROID
	FString RHIName = GDynamicRHI ? GDynamicRHI->GetName() : TEXT("Unknown");
	if (RHIName.Contains(TEXT("OpenGL")))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Applying OpenGL flip/mirror correction for captured screenshot"));

		Algo::Reverse(*Bitmap);

		for (int32 Y = 0; Y < ViewportSize.Y; ++Y)
		{
			int32 RowStart = Y * ViewportSize.X;
			int32 RowEnd = RowStart + ViewportSize.X - 1;
			for (int32 X = 0; X < ViewportSize.X / 2; ++X)
			{
				Swap((*Bitmap)[RowStart + X], (*Bitmap)[RowEnd - X]);
			}
		}
	}
	else
	{
		UE_LOG(LogSentrySdk, Log, TEXT("No flip/mirror correction required captured screenshot (Vulkan or other RHI)"));
	}
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	GetHighResScreenshotConfig().MergeMaskIntoAlpha(*Bitmap);
	TArray<uint8>* CompressedBitmap = new TArray<uint8>();
	FImageUtils::CompressImageArray(ViewportSize.X, ViewportSize.Y, *Bitmap, *CompressedBitmap);
#else
	GetHighResScreenshotConfig().MergeMaskIntoAlpha(*Bitmap, FIntRect());
	TArray64<uint8>* CompressedBitmap = new TArray64<uint8>();
	FImageUtils::PNGCompressImageArray(ViewportSize.X, ViewportSize.Y, *Bitmap, *CompressedBitmap);
#endif

	FFileHelper::SaveArrayToFile(*CompressedBitmap, *ScreenshotSavePath);

	delete Bitmap;
	delete CompressedBitmap;

	UE_LOG(LogSentrySdk, Log, TEXT("Screenshot saved to: %s"), *ScreenshotSavePath);

	return true;
}
