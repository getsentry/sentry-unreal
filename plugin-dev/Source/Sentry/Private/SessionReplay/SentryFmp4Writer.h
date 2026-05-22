// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_SESSION_REPLAY

/**
 * Fragmented MP4 (fMP4) writer for video-only H.264 streams.
 *
 * Produces an ISO/IEC 14496-12 compliant byte stream of the form:
 *   ftyp + moov                (init segment; track has empty sample tables)
 *   moof + mdat                (independent fragment 1)
 *   moof + mdat                (independent fragment 2)
 *   ...
 *
 * Each fragment must begin with an IDR (keyframe). Concatenating
 * `init_segment + last_N_complete_fragments` always yields a playable file,
 * which is the property the crash-video rotation strategy relies on.
 */
struct FSentryH264Sample
{
	// AVCC payload (length-prefixed NALUs, no Annex-B start codes).
	TArray<uint8> AvccBytes;

	// Sample duration in track timescale ticks.
	uint32 Duration;

	// Whether this is an IDR sample (the first sample of every fragment must be IDR).
	bool bIsKeyframe;
};

class FSentryFMP4Writer
{
public:
	// Track timescale (ticks per second). 90000 is the canonical H.264 choice.
	static constexpr uint32 TrackTimescale = 90000;

	// Default trackID; arbitrary but must match the track in init + fragments.
	static constexpr uint32 TrackId = 1;

	/**
	 * Build the init segment (ftyp + moov) for an H.264 video track of the given
	 * size. The SPS and PPS bytes must NOT include Annex-B start codes — raw NALU
	 * payload only (i.e. starting with the H.264 NAL header byte).
	 */
	static TArray<uint8> BuildInitSegment(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps);

	/**
	 * Build a single fragment (moof + mdat) carrying the supplied samples.
	 *
	 * SequenceNumber: 1-based, monotonically increasing across fragments.
	 * BaseMediaDecodeTime: decode time of the first sample, in TrackTimescale ticks.
	 */
	static TArray<uint8> BuildFragment(uint32 SequenceNumber, uint64 BaseMediaDecodeTime, const TArray<FSentryH264Sample>& Samples);

	/**
	 * Split an Annex-B byte stream into NALU payloads (start codes stripped).
	 */
	static TArray<TArray<uint8>> SplitAnnexB(const uint8* Data, int64 Size);

	/**
	 * Convert an Annex-B byte stream to AVCC (4-byte big-endian length prefix
	 * per NALU). SPS/PPS NALUs (types 7/8) are dropped from the output and
	 * appended to OutSps/OutPps instead, since they belong in the init
	 * segment's avcC box rather than inside mdat.
	 */
	static TArray<uint8> AnnexBToAvcc(const uint8* Data, int64 Size, TArray<uint8>* OutSps, TArray<uint8>* OutPps);

	// In-place big-endian byte poking helpers, exposed so the rotation
	// thread can patch tfdt offsets when rebasing serialized fragments.
	static void PatchU32(TArray<uint8>& Out, int32 Offset, uint32 V);
	static void PatchU64(TArray<uint8>& Out, int32 Offset, uint64 V);
	static uint64 ReadU64BE(const TArray<uint8>& Bytes, int32 Offset);
};

#endif // USE_SENTRY_SESSION_REPLAY
