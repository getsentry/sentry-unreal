// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryReplayFragmentAssembler.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentrySessionReplayRecorder.h"

FSentryReplayFragmentAssembler::FSentryReplayFragmentAssembler(FSentrySessionReplayRecorder& InRecorder)
	: Recorder(InRecorder)
{
}

void FSentryReplayFragmentAssembler::SetDimensions(uint32 InWidth, uint32 InHeight)
{
	Width = InWidth;
	Height = InHeight;
}

void FSentryReplayFragmentAssembler::AddAccessUnit(const uint8* AnnexB, int64 Size, bool bKeyframe, uint32 DurationTicks)
{
	if (AnnexB == nullptr || Size <= 0)
	{
		return;
	}

	TArray<uint8> Sps, Pps;
	TArray<uint8> Avcc = FSentryFMP4Writer::AnnexBToAvcc(AnnexB, Size, &Sps, &Pps);

	if (Sps.Num() > 0 && CachedSps.Num() == 0)
	{
		CachedSps = MoveTemp(Sps);
	}
	if (Pps.Num() > 0 && CachedPps.Num() == 0)
	{
		CachedPps = MoveTemp(Pps);
	}

	if (!bInitSegmentPublished && CachedSps.Num() > 0 && CachedPps.Num() > 0)
	{
		TArray<uint8> Init = FSentryFMP4Writer::BuildInitSegment(Width, Height, CachedSps, CachedPps);
		Recorder.OnInitSegmentReady(MoveTemp(Init));
		bInitSegmentPublished = true;
	}

	if (Avcc.Num() == 0)
	{
		return;
	}

	// Every fragment has to start on a keyframe, so close the previous one out
	// as soon as a new IDR shows up
	if (bKeyframe && CurrentSamples.Num() > 0)
	{
		Flush();
	}

	FSentryH264Sample Sample;
	Sample.AvccBytes = MoveTemp(Avcc);
	Sample.Duration = FMath::Max<uint32>(1, DurationTicks);

	if (CurrentSamples.Num() == 0)
	{
		CurrentFragmentDecodeTime = SampleClock;
	}
	SampleClock += Sample.Duration;

	CurrentSamples.Add(MoveTemp(Sample));

	// Every frame is independently decodable on the software path, so close the
	// fragment now rather than holding it open for a keyframe that is a whole
	// capture interval away
	if (bFlushEveryKeyframe && bKeyframe)
	{
		Flush();
	}
}

void FSentryReplayFragmentAssembler::Flush()
{
	if (CurrentSamples.Num() > 0 && bInitSegmentPublished)
	{
		const uint32 FrameCount = static_cast<uint32>(CurrentSamples.Num());
		uint64 DurationTicks = 0;
		for (const FSentryH264Sample& Sample : CurrentSamples)
		{
			DurationTicks += Sample.Duration;
		}
		TArray<uint8> Fragment = FSentryFMP4Writer::BuildFragment(NextFragmentSequence++, CurrentFragmentDecodeTime, CurrentSamples);
		Recorder.OnFragmentReady(MoveTemp(Fragment), FrameCount, DurationTicks);
	}
	CurrentSamples.Reset();
}

void FSentryReplayFragmentAssembler::Reset()
{
	CurrentSamples.Reset();
	CurrentFragmentDecodeTime = 0;
	NextFragmentSequence = 1;
	SampleClock = 0;
	CachedSps.Empty();
	CachedPps.Empty();
	bInitSegmentPublished = false;
}

#endif // USE_SENTRY_SESSION_REPLAY
