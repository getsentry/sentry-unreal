// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBackBufferCapture.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryDefines.h"
#include "SentryVideoEncoder.h"

#include "Framework/Application/SlateApplication.h"
#include "GlobalShader.h"
#include "HAL/PlatformTime.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "Widgets/SWindow.h"

#if !UE_VERSION_OLDER_THAN(5, 8, 0)
#include "Slate/SlateViewportProvider.h"
#endif

FSentryBackBufferCapture::FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder)
	: Encoder(InEncoder)
{
	EncoderPool.Slots.SetNum(FSentryVideoEncoder::MaxQueueDepth);
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
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: Slate not initialised, cannot hook backbuffer"));
		return false;
	}
	FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer();
	if (!Renderer)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: no Slate renderer"));
		return false;
	}

	BackBufferReadyHandle = Renderer->OnBackBufferReadyToPresent().AddRaw(
		this, &FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread);

	return true;
}

void FSentryBackBufferCapture::Stop()
{
	if (BackBufferReadyHandle.IsValid() && FSlateApplication::IsInitialized())
	{
		if (FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer())
		{
			Renderer->OnBackBufferReadyToPresent().Remove(BackBufferReadyHandle);
		}
		BackBufferReadyHandle.Reset();
	}

	// Ensure render thread is done with our textures before destruction
	FlushRenderingCommands();
	for (FTextureRHIRef& Slot : EncoderPool.Slots)
	{
		Slot.SafeRelease();
	}
	Scratch.Texture.SafeRelease();
	Converted.Texture.SafeRelease();
}

#if UE_VERSION_OLDER_THAN(5, 8, 0)
void FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer)
{
	CaptureBackBuffer_RenderThread(BackBuffer);
}
#else
void FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, ISlateViewportProvider& ViewportProvider)
{
	CaptureBackBuffer_RenderThread(ViewportProvider.GetBackBufferResource());
}
#endif

void FSentryBackBufferCapture::CaptureBackBuffer_RenderThread(const FTextureRHIRef& BackBuffer)
{
	check(IsInRenderingThread());

	if (!BackBuffer.IsValid() || Encoder.IsEncodingDisabled())
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	if (Now < NextCaptureTime)
	{
		return;
	}
	NextCaptureTime = Now + CapturePeriodSeconds;

	const uint32 W = BackBuffer->GetSizeX();
	const uint32 H = BackBuffer->GetSizeY();

	if (W == 0 || H == 0)
	{
		return;
	}

	constexpr ETextureCreateFlags SrvRt = ETextureCreateFlags::ShaderResource | ETextureCreateFlags::RenderTargetable;

	FTextureRHIRef ScratchTex = AcquireCachedTexture_RenderThread(Scratch, W, H, BackBuffer->GetFormat(),
		SrvRt, ERHIAccess::SRVGraphics, TEXT("SentrySessionReplayScratch"));

#if PLATFORM_MAC
	FTextureRHIRef ConvertedTex = AcquireCachedTexture_RenderThread(Converted, W, H, PF_B8G8R8A8,
		SrvRt, ERHIAccess::SRVGraphics, TEXT("SentrySessionReplayConverted"));
#endif

#if PLATFORM_MAC
	// VT requires pool slot to have CPUReadback flag so it can be read via Metal's getBytes().
	// Note: Metal forbids RT|CPUReadback on the same texture so an extra copy is needed (step 3)
	constexpr ETextureCreateFlags PoolFlags = ETextureCreateFlags::CPUReadback;
	constexpr ERHIAccess PoolInitialState = ERHIAccess::CPURead;
#else
	// NVENC reads from the pool slot directly. The external-memory flag exposes the
	// texture's GPU allocation to the encoder; SRV|RT lets the draw pass write into it.
	// The flag is RHI-specific: D3D (Windows) uses Shared, while Vulkan (Linux, or
	// Windows -vulkan) needs External so AVCodecs can import it through CUDA. This must
	// match how the engine's FVideoResourceRHI::Create chooses the flag per RHI, otherwise
	// the resource has no shareable handle and the encoder rejects every frame.
	const ETextureCreateFlags InteropFlag = (RHIGetInterfaceType() == ERHIInterfaceType::Vulkan)
												? ETextureCreateFlags::External
												: ETextureCreateFlags::Shared;
	const ETextureCreateFlags PoolFlags = InteropFlag | SrvRt;
	constexpr ERHIAccess PoolInitialState = ERHIAccess::SRVGraphics;
#endif

	FTextureRHIRef EncoderTex = AcquireTexturePoolSlot_RenderThread(EncoderPool, W, H, PF_B8G8R8A8,
		PoolFlags, PoolInitialState, TEXT("SentrySessionReplayCapture"));

	if (!ScratchTex.IsValid())
	{
		return;
	}

#if PLATFORM_MAC
	if (!ConvertedTex.IsValid())
	{
		return;
	}
#endif

	if (!EncoderTex.IsValid())
	{
		return;
	}

#if PLATFORM_MAC
	FTextureRHIRef DrawTarget = ConvertedTex;
#else
	FTextureRHIRef DrawTarget = EncoderTex;
#endif

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	// Step 1: hardware-copy backbuffer -> scratch (same format). Slate
	// backbuffers don't carry TexCreate_ShaderResource, so we can't bind
	// them directly as an SRV in the shader path below
	{
		FRHITransitionInfo Transitions[] = {
			FRHITransitionInfo(BackBuffer.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopySrc),
			FRHITransitionInfo(ScratchTex.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopyDest),
		};
		RHICmdList.Transition(MakeArrayView(Transitions, UE_ARRAY_COUNT(Transitions)));

		FRHICopyTextureInfo CopyInfo;
		CopyInfo.Size = FIntVector(static_cast<int32>(W), static_cast<int32>(H), 1);
		RHICmdList.CopyTexture(BackBuffer.GetReference(), ScratchTex.GetReference(), CopyInfo);

		RHICmdList.Transition(FRHITransitionInfo(ScratchTex.GetReference(), ERHIAccess::CopyDest, ERHIAccess::SRVGraphics));
	}

	// Step 2: AddDrawTexturePass scratch -> DrawTarget (BGRA8). Same-format fast
	// path is a hardware copy; format mismatch (HDR / 10-bit source) falls back
	// to the engine's built-in FDrawTexturePS pixel shader
	{
		FRDGBuilder GraphBuilder(RHICmdList);

		FRDGTextureRef RDGSource = RegisterExternalTexture(GraphBuilder, ScratchTex, TEXT("SentrySR_Scratch"));
		FRDGTextureRef RDGDest = RegisterExternalTexture(GraphBuilder, DrawTarget, TEXT("SentrySR_Dest"));

		FRDGDrawTextureInfo DrawInfo;
		DrawInfo.Size = FIntPoint(static_cast<int32>(W), static_cast<int32>(H));
		AddDrawTexturePass(GraphBuilder, GetGlobalShaderMap(GMaxRHIFeatureLevel), RDGSource, RDGDest, DrawInfo);

		GraphBuilder.Execute();
	}

#if PLATFORM_MAC
	// Step 3 (Mac only): hardware-copy Converted -> EncoderPool slot. Both are BGRA8 same-format
	{
		FRHITransitionInfo Transitions[] = {
			FRHITransitionInfo(DrawTarget.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopySrc),
			FRHITransitionInfo(EncoderTex.GetReference(), ERHIAccess::Unknown, ERHIAccess::CopyDest),
		};
		RHICmdList.Transition(MakeArrayView(Transitions, UE_ARRAY_COUNT(Transitions)));

		FRHICopyTextureInfo CopyInfo;
		CopyInfo.Size = FIntVector(static_cast<int32>(W), static_cast<int32>(H), 1);
		RHICmdList.CopyTexture(DrawTarget.GetReference(), EncoderTex.GetReference(), CopyInfo);
	}
#endif

#if PLATFORM_MAC
	RHICmdList.Transition(FRHITransitionInfo(EncoderTex.GetReference(), ERHIAccess::CopyDest, ERHIAccess::CPURead));
#else
	RHICmdList.Transition(FRHITransitionInfo(EncoderTex.GetReference(), ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
#endif

	Encoder.SubmitFrame(EncoderTex, Now);
}

FTextureRHIRef FSentryBackBufferCapture::AcquireCachedTexture_RenderThread(FCachedTexture& Cache, uint32 Width, uint32 Height, EPixelFormat Format,
	ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName)
{
	if (Width != Cache.Width || Height != Cache.Height || Format != Cache.Format || Flags != Cache.Flags)
	{
		Cache.Texture.SafeRelease();
		Cache.Width = Width;
		Cache.Height = Height;
		Cache.Format = Format;
		Cache.Flags = Flags;
	}

	if (!Cache.Texture.IsValid())
	{
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(DebugName)
											   .SetExtent(static_cast<int32>(Width), static_cast<int32>(Height))
											   .SetFormat(Format)
											   .SetFlags(Flags)
											   .SetInitialState(InitialState);
		Cache.Texture = RHICreateTexture(Desc);
	}

	return Cache.Texture;
}

FTextureRHIRef FSentryBackBufferCapture::AcquireTexturePoolSlot_RenderThread(FCachedTexturePool& Pool, uint32 Width, uint32 Height, EPixelFormat Format,
	ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName)
{
	if (Width != Pool.Width || Height != Pool.Height || Format != Pool.Format || Flags != Pool.Flags)
	{
		for (FTextureRHIRef& Slot : Pool.Slots)
		{
			Slot.SafeRelease();
		}
		Pool.Width = Width;
		Pool.Height = Height;
		Pool.Format = Format;
		Pool.Flags = Flags;
	}

	FTextureRHIRef* FreeSlot = nullptr;
	for (FTextureRHIRef& Slot : Pool.Slots)
	{
		if (Slot.GetRefCount() <= 1)
		{
			FreeSlot = &Slot;
			break;
		}
	}

	if (!FreeSlot)
	{
		return nullptr;
	}

	if (!FreeSlot->IsValid())
	{
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(DebugName)
											   .SetExtent(static_cast<int32>(Width), static_cast<int32>(Height))
											   .SetFormat(Format)
											   .SetFlags(Flags)
											   .SetInitialState(InitialState);
		*FreeSlot = RHICreateTexture(Desc);
	}

	return *FreeSlot;
}

#endif // USE_SENTRY_SESSION_REPLAY
