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

FSentryBackBufferCapture::FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder)
	: Encoder(InEncoder)
{
	TexturePool.SetNum(TexturePoolSize);
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
	for (FTextureRHIRef& Slot : TexturePool)
	{
		Slot.SafeRelease();
	}
	ScratchTexture.SafeRelease();
}

void FSentryBackBufferCapture::OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer)
{
	check(IsInRenderingThread());

	if (!BackBuffer.IsValid())
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

	const EPixelFormat SrcFormat = BackBuffer->GetFormat();

	FTextureRHIRef ScratchTex = AcquireScratchTexture_RenderThread(W, H, SrcFormat);
	FTextureRHIRef DestTexture = AcquireTexturePoolSlot_RenderThread(W, H);
	if (!ScratchTex.IsValid() || !DestTexture.IsValid())
	{
		return;
	}

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

	// Step 2: AddDrawTexturePass scratch -> pool (BGRA8). Same-format fast
	// path is a hardware copy; format mismatch (HDR / 10-bit source) falls
	// back to the engine's built-in FDrawTexturePS pixel shader
	{
		FRDGBuilder GraphBuilder(RHICmdList);

		FRDGTextureRef RDGSource = RegisterExternalTexture(GraphBuilder, ScratchTex, TEXT("SentrySR_Scratch"));
		FRDGTextureRef RDGDest = RegisterExternalTexture(GraphBuilder, DestTexture, TEXT("SentrySR_Dest"));

		FRDGDrawTextureInfo DrawInfo;
		DrawInfo.Size = FIntPoint(static_cast<int32>(W), static_cast<int32>(H));
		AddDrawTexturePass(GraphBuilder, GetGlobalShaderMap(GMaxRHIFeatureLevel), RDGSource, RDGDest, DrawInfo);

		GraphBuilder.Execute();
	}

	// Leave the destination in a shader-readable state for NVENC
	RHICmdList.Transition(FRHITransitionInfo(DestTexture.GetReference(), ERHIAccess::Unknown, ERHIAccess::SRVGraphics));

	Encoder.SubmitFrame(DestTexture);
}

FTextureRHIRef FSentryBackBufferCapture::AcquireTexturePoolSlot_RenderThread(uint32 Width, uint32 Height)
{
	if (Width != TexturePoolWidth || Height != TexturePoolHeight)
	{
		for (FTextureRHIRef& Slot : TexturePool)
		{
			Slot.SafeRelease();
		}
		TexturePoolWidth = Width;
		TexturePoolHeight = Height;
		NextTexturePoolSlot = 0;
	}

	FTextureRHIRef& Slot = TexturePool[NextTexturePoolSlot];
	NextTexturePoolSlot = (NextTexturePoolSlot + 1) % TexturePoolSize;

	if (!Slot.IsValid())
	{
		// Texture flags follow what AVCodecs expects for its NVENC resources
		// on D3D (see Engine/.../AVCodecsCoreRHI/.../VideoResourceRHI.cpp):
		// Shared (cross-process for NVENC interop) + ShaderResource +
		// RenderTargetable. Format is always PF_B8G8R8A8 because that's what
		// NVENC D3D12 requires; source-format conversion happens at draw time
		const ETextureCreateFlags CreateFlags = ETextureCreateFlags::Shared | ETextureCreateFlags::ShaderResource | ETextureCreateFlags::RenderTargetable;
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("SentrySessionReplayCapture"))
											   .SetExtent(static_cast<int32>(Width), static_cast<int32>(Height))
											   .SetFormat(PF_B8G8R8A8)
											   .SetFlags(CreateFlags)
											   .SetInitialState(ERHIAccess::SRVGraphics);
		Slot = RHICreateTexture(Desc);
	}
	return Slot;
}

FTextureRHIRef FSentryBackBufferCapture::AcquireScratchTexture_RenderThread(uint32 Width, uint32 Height, EPixelFormat Format)
{
	if (Width != ScratchWidth || Height != ScratchHeight || Format != ScratchFormat)
	{
		ScratchTexture.SafeRelease();
		ScratchWidth = Width;
		ScratchHeight = Height;
		ScratchFormat = Format;
	}

	if (!ScratchTexture.IsValid())
	{
		// Mirror the backbuffer's format, but add ShaderResource so
		// AddDrawTexturePass can sample it. RenderTargetable lets the same
		// scratch double as a draw target in the same-format fast path
		// inside AddDrawTexturePass (it still ends up as a HW copy)
		const ETextureCreateFlags CreateFlags = ETextureCreateFlags::ShaderResource | ETextureCreateFlags::RenderTargetable;
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("SentrySessionReplayScratch"))
											   .SetExtent(static_cast<int32>(Width), static_cast<int32>(Height))
											   .SetFormat(Format)
											   .SetFlags(CreateFlags)
											   .SetInitialState(ERHIAccess::SRVGraphics);
		ScratchTexture = RHICreateTexture(Desc);
	}
	return ScratchTexture;
}

#endif // USE_SENTRY_SESSION_REPLAY
