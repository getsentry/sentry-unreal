// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryScreenshotUtils.h"
#include "SentryDefines.h"

#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "UnrealClient.h"
#include "Misc/FileHelper.h"
#include "Engine/GameViewportClient.h"

bool SentryScreenshotUtils::CaptureScreenshot(const FString& ScreenshotSavePath)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("GameViewport required for screenshot capturing is not valid"));
		return false;
	}

	FViewport* Viewport = GEngine->GameViewport->Viewport;
	if (!Viewport)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Viewport required for screenshot capturing is not valid"));
		return false;
	}

	TArray<FColor> Bitmap;
	if (Viewport->ReadPixels(Bitmap))
	{
		for (FColor& Color : Bitmap)
		{
			Color.A = 255;
		}

		TArray64<uint8> CompressedBitmap;
		FImageUtils::PNGCompressImageArray(Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y, Bitmap, CompressedBitmap);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *ScreenshotSavePath);

		UE_LOG(LogSentrySdk, Log, TEXT("Screenshot saved to: %s"), *ScreenshotSavePath);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to read pixels from viewport"));
		return false;
	}

	return true;
}
