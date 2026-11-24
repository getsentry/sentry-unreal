// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

#include "SentrySettings.h"
#include "SentryVariant.h"

#include "Containers/Ticker.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class ISentrySubsystem;
class FSentryGameStateFileWriter;
class FSentryGameStateInputProcessor;
class FSentryGameStateRecorder;
class FSentryGameStateSampler;
class UWorld;
struct FKey;

/**
 * Records a rolling window of game state that the platform SDK attaches to crash events.
 *
 * Samples the game world on the game thread and periodically writes the window to a file
 * on a background thread. Only the first local player's world is captured when several
 * PIE instances run in one process.
 */
class FSentryGameStateIntegration final : public ISentryIntegration
{
public:
	explicit FSentryGameStateIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);
	virtual ~FSentryGameStateIntegration() override;

	static const TCHAR* StaticName() { return TEXT("GameStateCapture"); }

	virtual const TCHAR* GetName() const override { return StaticName(); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

	void AddTrackedActor(AActor* Actor, const FString& Id, const FString& Tag);
	void RemoveTrackedActor(AActor* Actor);

	void SetAttribute(const FString& Key, const FSentryVariant& Value);
	void RemoveAttribute(const FString& Key);

private:
	static constexpr double FlushIntervalSeconds = 1.0;

	bool OnTick(float DeltaTime);

	void OnWorldChanged(UWorld* World);
	void OnEndPlayMap();
	void OnClick(const FVector2D& ScreenPosition, const FKey& Button);

	void RegisterInputProcessor();
	void UnregisterInputProcessor();

	void BeginCapture(UWorld* World);
	void EndCapture();

	void UpdateHeader(UWorld* World);

	void FlushToFile();

	double GetCaptureTime() const;

	UWorld* FindGameWorld() const;

	TSharedRef<ISentrySubsystem> NativeImpl;

	FSentryGameStateCaptureOptions Options;

	FString FilePath;

	FString CaptureId;

	TUniquePtr<FSentryGameStateRecorder> Recorder;
	TUniquePtr<FSentryGameStateSampler> Sampler;
	TUniquePtr<FSentryGameStateFileWriter> FileWriter;

	TWeakObjectPtr<UWorld> CapturedWorld;

	bool bCapturing = false;
	double CaptureStartSeconds = 0.0;
	double LastFlushSeconds = 0.0;
	int64 CaptureStartUnixTime = 0;

	TSharedPtr<FSentryGameStateInputProcessor> InputProcessor;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FDelegateHandle TickerHandle;
#else
	FTSTicker::FDelegateHandle TickerHandle;
#endif

	FDelegateHandle EndPlayMapHandle;
};
