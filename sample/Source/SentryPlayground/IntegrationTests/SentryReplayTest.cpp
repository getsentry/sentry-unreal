// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryReplayTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryVariant.h"

#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

namespace
{

// Claimed properties of the staged clip used to build the replay envelope
constexpr int32 ClipDurationMs = 5000;
constexpr int32 ClipWidth = 1280;
constexpr int32 ClipHeight = 720;
constexpr int32 ClipFrameRate = 30;
constexpr int32 ClipFrameCount = ClipDurationMs / 1000 * ClipFrameRate;
constexpr int32 ClipSizeBytes = 64 * 1024;

// Mirrors the directories the SDK scans for replay files which are not exposed publicly:
// FAppleSentrySubsystem::GetReplayPath (cocoa) and FGenericPlatformSentrySubsystem::GetDatabasePath (sentry-native)
FString GetReplaysDir(const USentrySettings* Settings)
{
#if PLATFORM_IOS
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryReplays")));
#elif PLATFORM_MAC
	if (!Settings->UseNativeBackend)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryReplays")));
	}
#endif

	const FString DatabaseParentPath = Settings->DatabaseLocation == ESentryDatabaseLocation::ProjectDirectory
										   ? FPaths::ProjectDir()
										   : FPaths::ProjectUserDir();

	return FPaths::ConvertRelativePathToFull(FPaths::Combine(DatabaseParentPath, TEXT(".sentry-native"), TEXT("replays")));
}

} // namespace

void FSentryReplayTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	const FString ReplaysDir = GetReplaysDir(Settings);
	const FString ReplayId = FGuid::NewGuid().ToString(EGuidFormats::Digits).ToLower();

	// Stage the clip with filler bytes
	TArray<uint8> ClipBytes;
	ClipBytes.SetNumUninitialized(ClipSizeBytes);
	for (int32 i = 0; i < ClipBytes.Num(); ++i)
	{
		ClipBytes[i] = static_cast<uint8>(i % 251);
	}

	const FString ClipPath = FPaths::Combine(ReplaysDir, FString::Printf(TEXT("replay-%s.mp4"), *ReplayId));
	if (!FFileHelper::SaveArrayToFile(ClipBytes, *ClipPath))
	{
		CompleteWithResult(false, FString::Printf(TEXT("Failed to stage replay clip at %s"), *ClipPath));
		return;
	}

	// Stage the metadata sidecar; key names mirror the FSentryReplayInfo properties
	const double EndTimestampSec = static_cast<double>(FDateTime::UtcNow().ToUnixTimestamp());
	const double StartTimestampSec = EndTimestampSec - ClipDurationMs / 1000.0;

	const FString Sidecar = FString::Printf(
		TEXT("{\"replayId\":\"%s\",\"replayType\":\"buffer\",\"segmentId\":0,")
			TEXT("\"startTimestampSec\":%.3f,\"endTimestampSec\":%.3f,")
				TEXT("\"width\":%d,\"height\":%d,\"durationMs\":%d,\"sizeBytes\":%d,\"frameCount\":%d,\"frameRate\":%d}"),
		*ReplayId, StartTimestampSec, EndTimestampSec,
		ClipWidth, ClipHeight, ClipDurationMs, ClipSizeBytes, ClipFrameCount, ClipFrameRate);

	const FString SidecarPath = FPaths::Combine(ReplaysDir, FString::Printf(TEXT("replay-%s.json"), *ReplayId));
	if (!FFileHelper::SaveStringToFile(Sidecar, *SidecarPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		CompleteWithResult(false, FString::Printf(TEXT("Failed to stage replay metadata at %s"), *SidecarPath));
		return;
	}

	// Link the staged replay to the crash event; when the recorder is active it does the same at init
	Subsystem->SetContext(TEXT("replay"), { { TEXT("replay_id"), FSentryVariant(ReplayId) } });

	// Add breadcrumbs shortly before the crash so they fall inside the replay window;
	// the SDK is expected to embed them in the replay recording where the integration
	// test script can verify them (message/category values are asserted in CI)
	Subsystem->AddBreadcrumbWithParams(TEXT("Replay test breadcrumb"), TEXT("replay.test"), TEXT("default"), {}, ESentryLevel::Info);
	Subsystem->AddBreadcrumbWithParams(TEXT("Replay test breadcrumb with data"), TEXT("replay.test"), TEXT("default"),
		{ { TEXT("key"), FSentryVariant(TEXT("value")) } }, ESentryLevel::Warning);

	// Because we don't get the real crash event ID, create a fake one and set it as a tag
	// This tag is then used by integration test script in CI to fetch the event
	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
	UE_LOG(LogSentrySample, Log, TEXT("REPLAY_CAPTURED: %s\n"), *ReplayId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
	UE_LOG(LogSentrySample, Display, TEXT("REPLAY_CAPTURED: %s\n"), *ReplayId);
#endif

	// Flush logs to ensure output is captured before crash
	GLog->Flush();

	Subsystem->SetTag(TEXT("test.crash_id"), EventId);

	USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType::NullPointer);
}
