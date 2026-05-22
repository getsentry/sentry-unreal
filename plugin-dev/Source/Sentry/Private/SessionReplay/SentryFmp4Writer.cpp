// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryFMP4Writer.h"

#if USE_SENTRY_SESSION_REPLAY

namespace SentryFMP4Detail
{
// ---------------------------------------------------------------------------
// Low-level byte writers and ISO/IEC 14496-12 box framing primitives.
// All multi-byte integers are big-endian.
// ---------------------------------------------------------------------------

static void WriteU8(TArray<uint8>& Out, uint8 V)
{
	Out.Add(V);
}
static void WriteU16(TArray<uint8>& Out, uint16 V)
{
	Out.Add(static_cast<uint8>((V >> 8) & 0xFF));
	Out.Add(static_cast<uint8>(V & 0xFF));
}
static void WriteU24(TArray<uint8>& Out, uint32 V)
{
	Out.Add(static_cast<uint8>((V >> 16) & 0xFF));
	Out.Add(static_cast<uint8>((V >> 8) & 0xFF));
	Out.Add(static_cast<uint8>(V & 0xFF));
}
static void WriteU32(TArray<uint8>& Out, uint32 V)
{
	Out.Add(static_cast<uint8>((V >> 24) & 0xFF));
	Out.Add(static_cast<uint8>((V >> 16) & 0xFF));
	Out.Add(static_cast<uint8>((V >> 8) & 0xFF));
	Out.Add(static_cast<uint8>(V & 0xFF));
}
static void WriteU64(TArray<uint8>& Out, uint64 V)
{
	WriteU32(Out, static_cast<uint32>((V >> 32) & 0xFFFFFFFF));
	WriteU32(Out, static_cast<uint32>(V & 0xFFFFFFFF));
}
static void WriteFourCC(TArray<uint8>& Out, const char* FourCC)
{
	Out.Add(static_cast<uint8>(FourCC[0]));
	Out.Add(static_cast<uint8>(FourCC[1]));
	Out.Add(static_cast<uint8>(FourCC[2]));
	Out.Add(static_cast<uint8>(FourCC[3]));
}
static void WriteBytes(TArray<uint8>& Out, const uint8* Data, int32 Size)
{
	Out.Append(Data, Size);
}

// ISO 14496-12 transformation matrix, identity (no scale/rotate/translate).
// Stored as 3x3 of 16.16 / 2.30 fixed-point: { 1, 0, 0,  0, 1, 0,  0, 0, 1 }.
static void WriteUnityMatrix(TArray<uint8>& Out)
{
	WriteU32(Out, 0x00010000);
	WriteU32(Out, 0);
	WriteU32(Out, 0);
	WriteU32(Out, 0);
	WriteU32(Out, 0x00010000);
	WriteU32(Out, 0);
	WriteU32(Out, 0);
	WriteU32(Out, 0);
	WriteU32(Out, 0x40000000);
}

// Wrap a payload as `[size:u32][type:fourcc][payload...]`.
static TArray<uint8> Box(const char* Type, const TArray<uint8>& Payload)
{
	TArray<uint8> Out;
	Out.Reserve(8 + Payload.Num());
	WriteU32(Out, static_cast<uint32>(8 + Payload.Num()));
	WriteFourCC(Out, Type);
	Out.Append(Payload);
	return Out;
}

// Full box: `[size:u32][type:fourcc][version:u8][flags:u24][payload...]`.
static TArray<uint8> FullBox(const char* Type, uint8 Version, uint32 Flags, const TArray<uint8>& Payload)
{
	TArray<uint8> Inner;
	Inner.Reserve(4 + Payload.Num());
	WriteU8(Inner, Version);
	WriteU24(Inner, Flags);
	Inner.Append(Payload);
	return Box(Type, Inner);
}

// ---------------------------------------------------------------------------
// Init segment: ftyp + moov subtree.
// Sample tables (stbl children) are intentionally empty — per-sample metadata
// lives in each fragment's trun, not the init segment.
// ---------------------------------------------------------------------------

static TArray<uint8> BuildFtyp()
{
	TArray<uint8> P;
	WriteFourCC(P, "iso5"); // major brand
	WriteU32(P, 512);		// minor version
	WriteFourCC(P, "iso5"); // compatible brands
	WriteFourCC(P, "iso6");
	WriteFourCC(P, "mp41");
	WriteFourCC(P, "isom");
	WriteFourCC(P, "avc1");
	return Box("ftyp", P);
}

static TArray<uint8> BuildMvhd()
{
	TArray<uint8> P;
	WriteU32(P, 0);			 // creation_time
	WriteU32(P, 0);			 // modification_time
	WriteU32(P, 1000);		 // timescale (1 ms)
	WriteU32(P, 0);			 // duration (unknown in fMP4)
	WriteU32(P, 0x00010000); // rate 1.0
	WriteU16(P, 0x0100);	 // volume 1.0
	P.AddZeroed(10);		 // reserved
	WriteUnityMatrix(P);	 // matrix
	P.AddZeroed(24);		 // pre_defined
	WriteU32(P, 2);			 // next_track_ID
	return FullBox("mvhd", 0, 0, P);
}

static TArray<uint8> BuildTkhd(uint32 Width, uint32 Height)
{
	TArray<uint8> P;
	WriteU32(P, 0);							 // creation_time
	WriteU32(P, 0);							 // modification_time
	WriteU32(P, FSentryFMP4Writer::TrackId); // track_ID
	WriteU32(P, 0);							 // reserved
	WriteU32(P, 0);							 // duration
	P.AddZeroed(8);							 // reserved (2 * uint32)
	WriteU16(P, 0);							 // layer
	WriteU16(P, 0);							 // alternate_group
	WriteU16(P, 0);							 // volume
	WriteU16(P, 0);							 // reserved
	WriteUnityMatrix(P);					 // matrix
	WriteU32(P, Width << 16);				 // width (16.16 fixed-point)
	WriteU32(P, Height << 16);				 // height (16.16 fixed-point)
	// flags 0x07 = track_enabled | in_movie | in_preview
	return FullBox("tkhd", 0, 0x000007, P);
}

static TArray<uint8> BuildMdhd()
{
	TArray<uint8> P;
	WriteU32(P, 0);									// creation_time
	WriteU32(P, 0);									// modification_time
	WriteU32(P, FSentryFMP4Writer::TrackTimescale); // timescale
	WriteU32(P, 0);									// duration
	WriteU16(P, 0x55C4);							// language ("und")
	WriteU16(P, 0);									// pre_defined
	return FullBox("mdhd", 0, 0, P);
}

static TArray<uint8> BuildHdlr()
{
	TArray<uint8> P;
	WriteU32(P, 0);			// pre_defined
	WriteFourCC(P, "vide"); // handler_type
	P.AddZeroed(12);		// reserved
	// name string, null-terminated
	const char* Name = "VideoHandler";
	WriteBytes(P, reinterpret_cast<const uint8*>(Name), 12);
	WriteU8(P, 0);
	return FullBox("hdlr", 0, 0, P);
}

static TArray<uint8> BuildVmhd()
{
	TArray<uint8> P;
	WriteU16(P, 0); // graphicsmode
	// opcolor (3 x uint16)
	WriteU16(P, 0);
	WriteU16(P, 0);
	WriteU16(P, 0);
	return FullBox("vmhd", 0, 0x000001, P);
}

static TArray<uint8> BuildDref()
{
	TArray<uint8> P;
	WriteU32(P, 1); // entry_count
	// "url " full box, flags=0x01 (self-contained, no URL string follows).
	// Manually framed instead of calling FullBox() because the payload is empty.
	TArray<uint8> UrlInner;
	WriteU8(UrlInner, 0);		  // version
	WriteU24(UrlInner, 0x000001); // flags
	P.Append(Box("url ", UrlInner));
	return FullBox("dref", 0, 0, P);
}

static TArray<uint8> BuildDinf()
{
	return Box("dinf", BuildDref());
}

static TArray<uint8> BuildAvcC(const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	// Fallback defaults if the SPS NALU is malformed/short: Baseline profile,
	// no constraint flags, level 3.1. The three bytes after the SPS NAL
	// header carry profile_idc / profile_compatibility / level_idc per
	// ISO/IEC 14496-15 §5.3.3.1.2.
	constexpr uint8 DefaultProfileIdc = 0x42; // Baseline
	constexpr uint8 DefaultProfileCompat = 0x00;
	constexpr uint8 DefaultLevelIdc = 0x1F; // 3.1

	TArray<uint8> P;
	WriteU8(P, 1);												// configurationVersion
	WriteU8(P, Sps.Num() >= 4 ? Sps[1] : DefaultProfileIdc);	// AVCProfileIndication
	WriteU8(P, Sps.Num() >= 4 ? Sps[2] : DefaultProfileCompat); // profile_compatibility
	WriteU8(P, Sps.Num() >= 4 ? Sps[3] : DefaultLevelIdc);		// AVCLevelIndication
	WriteU8(P, 0xFF);											// 6 bits reserved + lengthSizeMinusOne (3 = 4 bytes)
	WriteU8(P, 0xE1);											// 3 bits reserved + numOfSPS (1)
	WriteU16(P, static_cast<uint16>(Sps.Num()));				// SPS length
	WriteBytes(P, Sps.GetData(), Sps.Num());					// SPS bytes
	WriteU8(P, 1);												// numOfPPS
	WriteU16(P, static_cast<uint16>(Pps.Num()));				// PPS length
	WriteBytes(P, Pps.GetData(), Pps.Num());					// PPS bytes
	return Box("avcC", P);
}

static TArray<uint8> BuildAvc1(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.AddZeroed(6);							  // SampleEntry reserved
	WriteU16(P, 1);							  // data_reference_index
	WriteU16(P, 0);							  // VisualSampleEntry pre_defined
	WriteU16(P, 0);							  // reserved
	P.AddZeroed(12);						  // pre_defined (3 * uint32)
	WriteU16(P, static_cast<uint16>(Width));  // width
	WriteU16(P, static_cast<uint16>(Height)); // height
	WriteU32(P, 0x00480000);				  // horizresolution 72 dpi
	WriteU32(P, 0x00480000);				  // vertresolution 72 dpi
	WriteU32(P, 0);							  // reserved
	WriteU16(P, 1);							  // frame_count
	WriteU8(P, 0);							  // compressorname length (empty)
	P.AddZeroed(31);						  // compressorname padding (31 bytes)
	WriteU16(P, 0x0018);					  // depth (24 bits)
	WriteU16(P, 0xFFFF);					  // pre_defined (-1)
	P.Append(BuildAvcC(Sps, Pps));			  // avcC child box
	return Box("avc1", P);
}

static TArray<uint8> BuildStsd(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	WriteU32(P, 1); // entry_count
	P.Append(BuildAvc1(Width, Height, Sps, Pps));
	return FullBox("stsd", 0, 0, P);
}

// Empty stbl children (fragmented: track has no samples in init segment)
static TArray<uint8> BuildEmptyStts()
{
	TArray<uint8> P;
	WriteU32(P, 0); // entry_count
	return FullBox("stts", 0, 0, P);
}
static TArray<uint8> BuildEmptyStsc()
{
	TArray<uint8> P;
	WriteU32(P, 0);
	return FullBox("stsc", 0, 0, P);
}
static TArray<uint8> BuildEmptyStsz()
{
	TArray<uint8> P;
	WriteU32(P, 0); // sample_size (0 = per-sample sizes follow)
	WriteU32(P, 0); // sample_count
	return FullBox("stsz", 0, 0, P);
}
static TArray<uint8> BuildEmptyStco()
{
	TArray<uint8> P;
	WriteU32(P, 0);
	return FullBox("stco", 0, 0, P);
}

static TArray<uint8> BuildStbl(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.Append(BuildStsd(Width, Height, Sps, Pps));
	P.Append(BuildEmptyStts());
	P.Append(BuildEmptyStsc());
	P.Append(BuildEmptyStsz());
	P.Append(BuildEmptyStco());
	return Box("stbl", P);
}

static TArray<uint8> BuildMinf(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.Append(BuildVmhd());
	P.Append(BuildDinf());
	P.Append(BuildStbl(Width, Height, Sps, Pps));
	return Box("minf", P);
}

static TArray<uint8> BuildMdia(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.Append(BuildMdhd());
	P.Append(BuildHdlr());
	P.Append(BuildMinf(Width, Height, Sps, Pps));
	return Box("mdia", P);
}

static TArray<uint8> BuildTrak(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.Append(BuildTkhd(Width, Height));
	P.Append(BuildMdia(Width, Height, Sps, Pps));
	return Box("trak", P);
}

static TArray<uint8> BuildTrex()
{
	TArray<uint8> P;
	WriteU32(P, FSentryFMP4Writer::TrackId); // track_ID
	WriteU32(P, 1);							 // default_sample_description_index
	WriteU32(P, 0);							 // default_sample_duration
	WriteU32(P, 0);							 // default_sample_size
	WriteU32(P, 0);							 // default_sample_flags
	return FullBox("trex", 0, 0, P);
}

static TArray<uint8> BuildMvex()
{
	return Box("mvex", BuildTrex());
}

static TArray<uint8> BuildMoov(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> P;
	P.Append(BuildMvhd());
	P.Append(BuildTrak(Width, Height, Sps, Pps));
	P.Append(BuildMvex());
	return Box("moov", P);
}

// ---------------------------------------------------------------------------
// Fragment: moof + mdat subtree.
// Each fragment is self-describing — it carries its own mini sample table
// (trun) with sizes/durations/keyframe flags. Players can stop at any
// fragment boundary and the file remains valid.
// ---------------------------------------------------------------------------

static TArray<uint8> BuildMfhd(uint32 SequenceNumber)
{
	TArray<uint8> P;
	WriteU32(P, SequenceNumber);
	return FullBox("mfhd", 0, 0, P);
}

static TArray<uint8> BuildTfhd()
{
	TArray<uint8> P;
	WriteU32(P, FSentryFMP4Writer::TrackId);
	// flags = 0x020000 (default-base-is-moof). Per-sample fields come from trun.
	return FullBox("tfhd", 0, 0x020000, P);
}

static TArray<uint8> BuildTfdt(uint64 BaseMediaDecodeTime)
{
	TArray<uint8> P;
	WriteU64(P, BaseMediaDecodeTime);
	return FullBox("tfdt", 1, 0, P);
}

// Sample flags for a keyframe: does not depend on others (sample_depends_on=2),
// is a sync sample. Applied via trun's first_sample_flags — fragments always
// start with an IDR.
static constexpr uint32 KeyframeSampleFlags = 0x02000000;

static TArray<uint8> BuildTrunWithData(int32 DataOffsetFromMoofStart, const TArray<FSentryH264Sample>& Samples)
{
	const uint32 Flags =
		0x000001 | // data-offset-present
		0x000004 | // first-sample-flags-present
		0x000100 | // sample-duration-present
		0x000200;  // sample-size-present

	TArray<uint8> P;
	WriteU32(P, static_cast<uint32>(Samples.Num()));		   // sample_count
	WriteU32(P, static_cast<uint32>(DataOffsetFromMoofStart)); // data_offset (patched by caller)
	WriteU32(P, KeyframeSampleFlags);						   // first_sample_flags
	for (const FSentryH264Sample& S : Samples)
	{
		WriteU32(P, S.Duration);
		WriteU32(P, static_cast<uint32>(S.AvccBytes.Num()));
	}
	return FullBox("trun", 0, Flags, P);
}
} // namespace SentryFMP4Detail

using namespace SentryFMP4Detail;

TArray<uint8> FSentryFMP4Writer::BuildInitSegment(uint32 Width, uint32 Height, const TArray<uint8>& Sps, const TArray<uint8>& Pps)
{
	TArray<uint8> Out;
	Out.Append(BuildFtyp());
	Out.Append(BuildMoov(Width, Height, Sps, Pps));
	return Out;
}

TArray<uint8> FSentryFMP4Writer::BuildFragment(uint32 SequenceNumber, uint64 BaseMediaDecodeTime, const TArray<FSentryH264Sample>& Samples)
{
	// Compute mdat size up front so we know the trun data_offset.
	int64 MdatBodySize = 0;
	for (const FSentryH264Sample& S : Samples)
	{
		MdatBodySize += S.AvccBytes.Num();
	}
	const int64 MdatBoxSize = 8 + MdatBodySize;

	// Build moof children. Trun uses a placeholder data_offset that's
	// patched once we know the final size of moof (data_offset is relative
	// to the moof start and points to the first sample byte in mdat).
	TArray<uint8> Mfhd = BuildMfhd(SequenceNumber);
	TArray<uint8> Tfhd = BuildTfhd();
	TArray<uint8> Tfdt = BuildTfdt(BaseMediaDecodeTime);
	TArray<uint8> Trun = BuildTrunWithData(/*placeholder*/ 0, Samples);

	// Trun lives at a deterministic byte offset inside Moof, given the sizes
	// of the boxes preceding it. Its layout (offsets relative to the trun
	// box's own start):
	//   0..3   size
	//   4..7   'trun'
	//   8      version
	//   9..11  flags
	//   12..15 sample_count
	//   16..19 data_offset      <-- patched below
	//   20..23 first_sample_flags
	//   ...per-sample entries
	const int32 TrunStartInMoof = 8 /*moof hdr*/ + Mfhd.Num() + 8 /*traf hdr*/ + Tfhd.Num() + Tfdt.Num();
	const int32 DataOffsetFieldIndex = TrunStartInMoof + 16;

	// Compose traf body + moof body.
	TArray<uint8> TrafBody;
	TrafBody.Append(Tfhd);
	TrafBody.Append(Tfdt);
	TrafBody.Append(Trun);

	TArray<uint8> MoofBody;
	MoofBody.Append(Mfhd);
	MoofBody.Append(Box("traf", TrafBody));
	TArray<uint8> Moof = Box("moof", MoofBody);

	// Now that the moof's final size is known, patch trun.data_offset =
	// moof box size + 8 (mdat header).
	const uint32 DataOffset = static_cast<uint32>(Moof.Num() + 8);
	FSentryFMP4Writer::PatchU32(Moof, DataOffsetFieldIndex, DataOffset);

	// Build mdat header + body
	TArray<uint8> Out;
	Out.Reserve(Moof.Num() + MdatBoxSize);
	Out.Append(Moof);
	WriteU32(Out, static_cast<uint32>(MdatBoxSize));
	WriteFourCC(Out, "mdat");
	for (const FSentryH264Sample& S : Samples)
	{
		Out.Append(S.AvccBytes);
	}
	return Out;
}

TArray<TArray<uint8>> FSentryFMP4Writer::SplitAnnexB(const uint8* Data, int64 Size)
{
	TArray<TArray<uint8>> Out;
	if (!Data || Size < 4)
	{
		return Out;
	}

	auto IsStart3 = [](const uint8* P, int64 i, int64 N)
	{
		return i + 3 <= N && P[i] == 0 && P[i + 1] == 0 && P[i + 2] == 1;
	};
	auto IsStart4 = [](const uint8* P, int64 i, int64 N)
	{
		return i + 4 <= N && P[i] == 0 && P[i + 1] == 0 && P[i + 2] == 0 && P[i + 3] == 1;
	};

	int64 i = 0;
	int64 NaluStart = -1;
	while (i < Size)
	{
		int64 StartLen = 0;
		if (IsStart4(Data, i, Size))
		{
			StartLen = 4;
		}
		else if (IsStart3(Data, i, Size))
		{
			StartLen = 3;
		}

		if (StartLen > 0)
		{
			if (NaluStart >= 0)
			{
				const int64 NaluEnd = i;
				if (NaluEnd > NaluStart)
				{
					TArray<uint8> Nalu;
					Nalu.Append(Data + NaluStart, NaluEnd - NaluStart);
					Out.Add(MoveTemp(Nalu));
				}
			}
			i += StartLen;
			NaluStart = i;
		}
		else
		{
			++i;
		}
	}
	if (NaluStart >= 0 && NaluStart < Size)
	{
		TArray<uint8> Nalu;
		Nalu.Append(Data + NaluStart, Size - NaluStart);
		Out.Add(MoveTemp(Nalu));
	}
	return Out;
}

TArray<uint8> FSentryFMP4Writer::AnnexBToAvcc(const uint8* Data, int64 Size, TArray<uint8>* OutSps, TArray<uint8>* OutPps)
{
	TArray<uint8> Out;
	const TArray<TArray<uint8>> Nalus = SplitAnnexB(Data, Size);
	for (const TArray<uint8>& Nalu : Nalus)
	{
		if (Nalu.Num() == 0)
		{
			continue;
		}
		const uint8 NalType = Nalu[0] & 0x1F;
		if (NalType == 7 /*SPS*/)
		{
			if (OutSps && OutSps->Num() == 0)
			{
				*OutSps = Nalu;
			}
			continue;
		}
		if (NalType == 8 /*PPS*/)
		{
			if (OutPps && OutPps->Num() == 0)
			{
				*OutPps = Nalu;
			}
			continue;
		}
		if (NalType == 9 /*Access unit delimiter*/)
		{
			// Skip AUDs; not needed in fMP4 and confuses some players.
			continue;
		}

		WriteU32(Out, static_cast<uint32>(Nalu.Num()));
		Out.Append(Nalu);
	}
	return Out;
}

void FSentryFMP4Writer::PatchU32(TArray<uint8>& Out, int32 Offset, uint32 V)
{
	check(Offset + 4 <= Out.Num());
	Out[Offset + 0] = static_cast<uint8>((V >> 24) & 0xFF);
	Out[Offset + 1] = static_cast<uint8>((V >> 16) & 0xFF);
	Out[Offset + 2] = static_cast<uint8>((V >> 8)  & 0xFF);
	Out[Offset + 3] = static_cast<uint8>( V        & 0xFF);
}

void FSentryFMP4Writer::PatchU64(TArray<uint8>& Out, int32 Offset, uint64 V)
{
	check(Offset + 8 <= Out.Num());
	for (int32 b = 0; b < 8; ++b)
	{
		Out[Offset + (7 - b)] = static_cast<uint8>(V & 0xFF);
		V >>= 8;
	}
}

uint64 FSentryFMP4Writer::ReadU64BE(const TArray<uint8>& Bytes, int32 Offset)
{
	if (Bytes.Num() < Offset + 8)
	{
		return 0;
	}
	uint64 V = 0;
	for (int32 b = 0; b < 8; ++b)
	{
		V = (V << 8) | static_cast<uint64>(Bytes[Offset + b]);
	}
	return V;
}

#endif // USE_SENTRY_SESSION_REPLAY
