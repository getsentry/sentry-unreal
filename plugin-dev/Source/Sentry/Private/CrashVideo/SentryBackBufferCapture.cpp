// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBackBufferCapture.h"

#if USE_SENTRY_CRASH_VIDEO

#include "SentryDefines.h"
#include "SentryVideoEncoder.h"

#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformTime.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RenderingThread.h"
#include "Widgets/SWindow.h"

FSentryBackBufferCapture::FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder)
	: Encoder(InEncoder)
{
	CapturePeriodSeconds = 1.0 / static_cast<double>(FMath::Max(1u, Encoder.GetFramerate()));
}

FSentryBackBufferCapture::~FSentryBackBufferCapture()
{
	Stop();
}

bool FSentryBackBufferCapture::Start()
{
	check(IsInGameThread());

	if (!FSlateApplication::IsInitialized())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: Slate not initialised, cannot hook backbuffer"));
		return false;
	}
	FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer();
	if (!Renderer)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: no Slate renderer"));
		return false;
	}

	DelegateHandle = Renderer->OnBackBufferReadyToPresent().AddRaw(
		this, &FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread);

	return true;
}

void FSentryBackBufferCapture::Stop()
{
	if (DelegateHandle.IsValid() && FSlateApplication::IsInitialized())
	{
		if (FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer())
		{
			Renderer->OnBackBufferReadyToPresent().Remove(DelegateHandle);
		}
		DelegateHandle.Reset();
	}

	// Ensure render thread is done with our pool textures before destruction.
	FlushRenderingCommands();
	for (int32 i = 0; i < PoolSize; ++i)
	{
		Pool[i].SafeRelease();
	}
}

FTextureRHIRef FSentryBackBufferCapture::AcquirePoolTexture_RenderThread(uint32 Width, uint32 Height, EPixelFormat Format)
{
	if (Width != PoolWidth || Height != PoolHeight || Format != PoolFormat)
	{
		// Resolution / format changed — drop the existing pool. New textures get created below.
		for (int32 i = 0; i < PoolSize; ++i)
		{
			Pool[i].SafeRelease();
		}
		PoolWidth = Width;
		PoolHeight = Height;
		PoolFormat = Format;
		NextPoolIndex = 0;
	}

	FTextureRHIRef& Slot = Pool[NextPoolIndex];
	NextPoolIndex = (NextPoolIndex + 1) % PoolSize;

	if (!Slot.IsValid())
	{
		// Texture flags follow what AVCodecs expects for its NVENC resources
		// on D3D (see Engine/.../AVCodecsCoreRHI/.../VideoResourceRHI.cpp):
		// Shared (cross-process for NVENC interop) + ShaderResource +
		// RenderTargetable.
		const ETextureCreateFlags CreateFlags = ETextureCreateFlags::Shared | ETextureCreateFlags::ShaderResource | ETextureCreateFlags::RenderTargetable;
		const ERHIAccess InitialAccess = ERHIAccess::SRVGraphics;
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("SentryCrashVideoCapture"))
											   .SetExtent(static_cast<int32>(Width), static_cast<int32>(Height))
											   .SetFormat(Format)
											   .SetFlags(CreateFlags)
											   .SetInitialState(InitialAccess);
		Slot = RHICreateTexture(Desc);
	}
	return Slot;
}

void FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer)
{
	check(IsInRenderingThread());

	if (!BackBuffer.IsValid())
	{
		return;
	}

	// v1: accept every top-level window broadcast — game-only target, single
	// window in practice. A proper primary-window filter would need a game-
	// thread-cached SWindow ref because FSlateApplication accessors assert
	// they're called from the game thread.

	// Frame throttling
	const double Now = FPlatformTime::Seconds();
	if (Now < NextCaptureTime)
	{
		return;
	}
	NextCaptureTime = Now + CapturePeriodSeconds;

	const uint32 W = BackBuffer->GetSizeX();
	const uint32 H = BackBuffer->GetSizeY();
	const EPixelFormat SrcFormat = BackBuffer->GetFormat();
	if (W == 0 || H == 0)
	{
		return;
	}

	// NVENC D3D12 only accepts BGRA8 input (see AVCodecs'
	// VideoEncoderNVENCD3D12::ConvertFormat). HDR backbuffer formats
	// (PF_FloatRGBA / PF_A2B10G10R10) would need a format-converting copy
	// pass — out of scope for v1.
	if (SrcFormat != PF_B8G8R8A8)
	{
		if (!bUnsupportedFormatLogged)
		{
			UE_LOG(LogSentrySdk, Warning,
				TEXT("Crash video: backbuffer format %d isn't supported by NVENC. ")
					TEXT("Set r.DefaultBackBufferPixelFormat=0 in DefaultEngine.ini. ")
						TEXT("Full HDR backbuffer support is a planned improvement."),
				static_cast<int32>(SrcFormat));
			bUnsupportedFormatLogged = true;
		}
		return;
	}

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	FTextureRHIRef DestTexture = AcquirePoolTexture_RenderThread(W, H, SrcFormat);
	if (!DestTexture.IsValid())
	{
		return;
	}

	// Same-format CopyTexture. The backbuffer is in RTV at this point
	// (SlateRHIRenderer transitions it to Present after this delegate
	// returns); passing ERHIAccess::Unknown lets the RHI infer the source
	// state from its internal tracking.
	FRHITransitionInfo Transitions[] = {
		FRHITransitionInfo(BackBuffer.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopySrc),
		FRHITransitionInfo(DestTexture.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopyDest),
	};
	RHICmdList.Transition(MakeArrayView(Transitions, UE_ARRAY_COUNT(Transitions)));

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.Size = FIntVector(static_cast<int32>(W), static_cast<int32>(H), 1);
	RHICmdList.CopyTexture(BackBuffer.GetReference(), DestTexture.GetReference(), CopyInfo);

	// Leave the destination in a shader-readable state for NVENC.
	FRHITransitionInfo DestPostCopy(DestTexture.GetReference(), ERHIAccess::CopyDest, ERHIAccess::SRVGraphics);
	RHICmdList.Transition(DestPostCopy);

	Encoder.SubmitFrame(DestTexture);
}

#endif // USE_SENTRY_CRASH_VIDEO
