// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"

#include "SentryShaderSymbolsCommandlet.generated.h"

/**
 * Commandlet that prepares the shader symbols exported by a cook for GPU crash symbolication.
 *
 * For every shader format under `Saved/ShaderSymbols` it computes the NVIDIA Aftermath hash of each `.dxil` binary,
 * joins the shader names from `ShaderSymbols.info` and writes `sentry-shader-manifest.tsv` next to the files. With
 * `-Upload` it then uploads the binaries with sentry-cli. The same work runs automatically at the end of a cook on
 * UE 5.5+ when the `Upload shader symbols after cook` setting is enabled; the commandlet is the explicit alternative
 * for CI pipelines and older engine versions.
 *
 * Usage:
 *   UnrealEditor-Cmd.exe <Project>.uproject -run=SentryShaderSymbols [-Upload]
 *
 * Requires a cook with `r.Shaders.Symbols=1` (and `r.Shaders.SymbolsInfo=1` for shader and material names) on a
 * Windows machine, since only the engine's Aftermath library can compute the hash. Returns 0 on success.
 */
UCLASS()
class USentryShaderSymbolsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USentryShaderSymbolsCommandlet();

	virtual int32 Main(const FString& Params) override;
};
