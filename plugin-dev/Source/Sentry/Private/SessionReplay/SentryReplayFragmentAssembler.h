// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryFMP4Writer.h"

class FSentrySessionReplayRecorder;

/**
 * Turns a stream of encoded H.264 access units into fMP4 fragments.
 *
 * Both encoder backends produce Annex-B access units and need the exact same
 * downstream bookkeeping: cache the first SPS/PPS, publish the init segment
 * once, group samples between keyframes into fragments, and keep a running
 * decode clock so playback follows the real capture cadence.
 *
 * Not thread-safe: every method is expected to be called from the owning
 * encoder's worker thread.
 */
class FSentryReplayFragmentAssembler
{
public:
	explicit FSentryReplayFragmentAssembler(FSentrySessionReplayRecorder& InRecorder);

	// Dimensions written into the init segment. Must be set before the first
	// access unit arrives
	void SetDimensions(uint32 InWidth, uint32 InHeight);

	/**
	 * Closes a fragment after every keyframe instead of waiting for the next one.
	 *
	 * A fragment normally stays open until the following keyframe arrives, which
	 * costs one capture interval of latency. That is fine at 30 fps and ruinous at
	 * 1 fps, where it means the most recent second is never on disk - and a crash
	 * shortly after the first frame produces no clip at all. Backends that make
	 * every frame an IDR should enable this.
	 */
	void SetFlushEveryKeyframe(bool bInFlushEveryKeyframe) { bFlushEveryKeyframe = bInFlushEveryKeyframe; }

	/**
	 * Feeds one encoded access unit.
	 *
	 * @param AnnexB       access unit as an Annex-B byte stream
	 * @param Size         length of AnnexB in bytes
	 * @param bKeyframe    true when this access unit starts with an IDR; each
	 *                     fMP4 fragment must begin on one
	 * @param DurationTicks sample duration in FSentryFMP4Writer::TrackTimescale ticks
	 */
	void AddAccessUnit(const uint8* AnnexB, int64 Size, bool bKeyframe, uint32 DurationTicks);

	// Emits whatever samples have accumulated as a fragment
	void Flush();

	// Drops all state so the next access unit re-publishes an init segment.
	// Used when an encoder is torn down and reopened mid-session
	void Reset();

	bool HasPublishedInitSegment() const { return bInitSegmentPublished; }

private:
	FSentrySessionReplayRecorder& Recorder;

	uint32 Width = 0;
	uint32 Height = 0;

	TArray<FSentryH264Sample> CurrentSamples;
	uint64 CurrentFragmentDecodeTime = 0;
	uint32 NextFragmentSequence = 1;
	uint64 SampleClock = 0;

	TArray<uint8> CachedSps;
	TArray<uint8> CachedPps;
	bool bInitSegmentPublished = false;
	bool bFlushEveryKeyframe = false;
};

#endif // USE_SENTRY_SESSION_REPLAY
