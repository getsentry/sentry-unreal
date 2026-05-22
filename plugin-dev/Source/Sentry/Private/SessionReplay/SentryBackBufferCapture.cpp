// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBackBufferCapture.h"

#if USE_SENTRY_SESSION_REPLAY

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
	ScratchTexture.SafeRelease();
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
		// inside AddDrawTexturePass (it still ends up as a HW copy).
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

FTextureRHIRef FSentryBackBufferCapture::AcquirePoolTexture_RenderThread(uint32 Width, uint32 Height)
{
	if (Width != PoolWidth || Height != PoolHeight)
	{
		// Resolution changed — drop the existing pool. New textures get created below.
		for (int32 i = 0; i < PoolSize; ++i)
		{
			Pool[i].SafeRelease();
		}
		PoolWidth = Width;
		PoolHeight = Height;
		NextPoolIndex = 0;
	}

	FTextureRHIRef& Slot = Pool[NextPoolIndex];
	NextPoolIndex = (NextPoolIndex + 1) % PoolSize;

	if (!Slot.IsValid())
	{
		// Texture flags follow what AVCodecs expects for its NVENC resources
		// on D3D (see Engine/.../AVCodecsCoreRHI/.../VideoResourceRHI.cpp):
		// Shared (cross-process for NVENC interop) + ShaderResource +
		// RenderTargetable. Format is always PF_B8G8R8A8 because that's what
		// NVENC D3D12 requires; source-format conversion happens at draw time.
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

	FTextureRHIRef ScratchTex = AcquireScratchTexture_RenderThread(W, H, SrcFormat);
	FTextureRHIRef DestTexture = AcquirePoolTexture_RenderThread(W, H);
	if (!ScratchTex.IsValid() || !DestTexture.IsValid())
	{
		return;
	}

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	// Step 1: hardware-copy backbuffer -> scratch (same format). Slate
	// backbuffers don't carry TexCreate_ShaderResource, so we can't bind
	// them directly as an SRV in the shader path below.
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
	// back to the engine's built-in FDrawTexturePS pixel shader. The GPU
	// clamps values >1.0 on the BGRA8 write, so scene-referred HDR
	// highlights blow out to white — acceptable for crash diagnostics.
	{
		FRDGBuilder GraphBuilder(RHICmdList);

		FRDGTextureRef RDGSource = RegisterExternalTexture(GraphBuilder, ScratchTex, TEXT("SentrySR_Scratch"));
		FRDGTextureRef RDGDest   = RegisterExternalTexture(GraphBuilder, DestTexture, TEXT("SentrySR_Dest"));

		FRDGDrawTextureInfo DrawInfo;
		DrawInfo.Size = FIntPoint(static_cast<int32>(W), static_cast<int32>(H));
		AddDrawTexturePass(GraphBuilder, GetGlobalShaderMap(GMaxRHIFeatureLevel), RDGSource, RDGDest, DrawInfo);

		GraphBuilder.Execute();
	}

	// Leave the destination in a shader-readable state for NVENC.
	RHICmdList.Transition(FRHITransitionInfo(DestTexture.GetReference(), ERHIAccess::Unknown, ERHIAccess::SRVGraphics));

	Encoder.SubmitFrame(DestTexture);
}

#endif // USE_SENTRY_SESSION_REPLAY
