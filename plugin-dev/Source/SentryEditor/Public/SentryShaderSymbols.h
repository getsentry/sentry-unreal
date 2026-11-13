// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Collects the shader symbols the cook exports (`Saved/ShaderSymbols/<Format>/<hash>.dxil`),
 * writes a manifest keyed by the NVIDIA Aftermath shader hash and optionally uploads the binaries to Sentry.
 * Runs at the end of a cook (UE 5.5+) or manually via `-run=SentryShaderSymbols`.
 */
class FSentryShaderSymbols
{
public:
	static void RegisterCookHook();

	/** Writes a manifest for every shader format under `<ProjectSavedDir>/ShaderSymbols` and uploads the binaries when bUpload is set. Returns 0 on success. */
	static int32 Run(const FString& ProjectSavedDir, bool bUpload);

private:
	static bool WriteManifest(const FString& FormatDir, const TArray<FString>& DxilFiles, int32& OutShaderCount);
	static bool HashShader(const TArray<uint8>& Shader, uint64& OutHash);
	static TMap<FString, FString> ReadSymbolsInfo(const FString& FormatDir);
	static int32 Upload(const FString& FormatDir);

	static const TCHAR* const ManifestFileName;
};
