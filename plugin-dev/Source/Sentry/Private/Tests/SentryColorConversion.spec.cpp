// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#if defined(WITH_AUTOMATION_TESTS) && defined(USE_SENTRY_SESSION_REPLAY)

#include "SessionReplay/SentryColorConversion.h"

BEGIN_DEFINE_SPEC(SentryColorConversionSpec, "Sentry.SessionReplay.ColorConversion", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)

	// Fills a BGRA image with one flat colour and converts it, returning the
	// centre sample of each plane.
	void ConvertFlat(uint8 B, uint8 G, uint8 R, uint32 Width, uint32 Height, uint8 & OutY, uint8 & OutU, uint8 & OutV);

	// Fills a BGRA image with a horizontal split - left half colour A, right half
	// colour B - so a downscale can be checked for averaging as well as placement.
	void ConvertSplit(uint8 B0, uint8 G0, uint8 R0, uint8 B1, uint8 G1, uint8 R1,
		uint32 SrcW, uint32 SrcH, uint32 SplitCol, uint32 DstW, uint32 DstH,
		TArray<uint8> & OutY, int32 & OutYStride);

END_DEFINE_SPEC(SentryColorConversionSpec)

void SentryColorConversionSpec::ConvertFlat(uint8 B, uint8 G, uint8 R, uint32 Width, uint32 Height, uint8& OutY, uint8& OutU, uint8& OutV)
{
	// Deliberately pad the source stride so the conversion is exercised with a
	// pitch wider than the image, which is what RHI readbacks actually hand over
	const int32 SrcStride = static_cast<int32>(Width) * 4 + 64;

	TArray<uint8> Source;
	Source.SetNumUninitialized(SrcStride * static_cast<int32>(Height));
	FMemory::Memset(Source.GetData(), 0xCD, Source.Num());

	for (uint32 Row = 0; Row < Height; ++Row)
	{
		uint8* RowPtr = Source.GetData() + static_cast<int64>(Row) * SrcStride;
		for (uint32 Col = 0; Col < Width; ++Col)
		{
			RowPtr[Col * 4 + 0] = B;
			RowPtr[Col * 4 + 1] = G;
			RowPtr[Col * 4 + 2] = R;
			RowPtr[Col * 4 + 3] = 0xFF;
		}
	}

	const int32 YStride = static_cast<int32>(Width);
	const int32 CStride = static_cast<int32>((Width + 1) / 2);
	const int32 ChromaHeight = static_cast<int32>((Height + 1) / 2);

	TArray<uint8> PlaneY, PlaneU, PlaneV;
	PlaneY.SetNumZeroed(YStride * static_cast<int32>(Height));
	PlaneU.SetNumZeroed(CStride * ChromaHeight);
	PlaneV.SetNumZeroed(CStride * ChromaHeight);

	FSentryColorConversion::BgraToI420(Source.GetData(), SrcStride, Width, Height,
		PlaneY.GetData(), PlaneU.GetData(), PlaneV.GetData(), YStride, CStride);

	OutY = PlaneY[(static_cast<int64>(Height) / 2) * YStride + Width / 2];
	OutU = PlaneU[(static_cast<int64>(ChromaHeight) / 2) * CStride + (CStride / 2)];
	OutV = PlaneV[(static_cast<int64>(ChromaHeight) / 2) * CStride + (CStride / 2)];
}

void SentryColorConversionSpec::ConvertSplit(uint8 B0, uint8 G0, uint8 R0, uint8 B1, uint8 G1, uint8 R1,
	uint32 SrcW, uint32 SrcH, uint32 SplitCol, uint32 DstW, uint32 DstH,
	TArray<uint8>& OutY, int32& OutYStride)
{
	const int32 SrcStride = static_cast<int32>(SrcW) * 4 + 32;

	TArray<uint8> Source;
	Source.SetNumUninitialized(SrcStride * static_cast<int32>(SrcH));
	FMemory::Memset(Source.GetData(), 0xCD, Source.Num());

	for (uint32 Row = 0; Row < SrcH; ++Row)
	{
		uint8* RowPtr = Source.GetData() + static_cast<int64>(Row) * SrcStride;
		for (uint32 Col = 0; Col < SrcW; ++Col)
		{
			const bool bLeft = Col < SplitCol;
			RowPtr[Col * 4 + 0] = bLeft ? B0 : B1;
			RowPtr[Col * 4 + 1] = bLeft ? G0 : G1;
			RowPtr[Col * 4 + 2] = bLeft ? R0 : R1;
			RowPtr[Col * 4 + 3] = 0xFF;
		}
	}

	OutYStride = static_cast<int32>(DstW);
	const int32 CStride = static_cast<int32>((DstW + 1) / 2);
	const int32 ChromaHeight = static_cast<int32>((DstH + 1) / 2);

	TArray<uint8> PlaneU, PlaneV;
	OutY.SetNumZeroed(OutYStride * static_cast<int32>(DstH));
	PlaneU.SetNumZeroed(CStride * ChromaHeight);
	PlaneV.SetNumZeroed(CStride * ChromaHeight);

	FSentryColorConversion::BgraToI420Scaled(Source.GetData(), SrcStride, SrcW, SrcH, DstW, DstH,
		OutY.GetData(), PlaneU.GetData(), PlaneV.GetData(), OutYStride, CStride);
}

void SentryColorConversionSpec::Define()
{
	Describe("BGRA to I420", [this]()
	{
		// The encoder signals BT.709 limited range in the SPS VUI. If these
		// expectations drift, replays still play - they just look washed out or
		// crushed - so pin the range explicitly.
		It("should map black to the limited-range floor", [this]()
		{
			uint8 Y = 0, U = 0, V = 0;
			ConvertFlat(0, 0, 0, 64, 64, Y, U, V);

			TestEqual("luma", static_cast<int32>(Y), 16);
			TestEqual("Cb", static_cast<int32>(U), 128);
			TestEqual("Cr", static_cast<int32>(V), 128);
		});

		It("should map white to the limited-range ceiling", [this]()
		{
			uint8 Y = 0, U = 0, V = 0;
			ConvertFlat(255, 255, 255, 64, 64, Y, U, V);

			TestEqual("luma", static_cast<int32>(Y), 235);
			TestEqual("Cb", static_cast<int32>(U), 128);
			TestEqual("Cr", static_cast<int32>(V), 128);
		});

		It("should push Cr high and Cb low for pure red", [this]()
		{
			uint8 Y = 0, U = 0, V = 0;
			ConvertFlat(0, 0, 255, 64, 64, Y, U, V);

			// BT.709: Y ~ 0.1826 * 255 + 16 ~ 62
			TestTrue("luma near 62", FMath::Abs(static_cast<int32>(Y) - 62) <= 1);
			TestTrue("Cb below neutral", U < 128);
			TestTrue("Cr at the ceiling", V >= 239);
		});

		It("should push Cb high for pure blue", [this]()
		{
			uint8 Y = 0, U = 0, V = 0;
			ConvertFlat(255, 0, 0, 64, 64, Y, U, V);

			TestTrue("Cb at the ceiling", U >= 239);
			TestTrue("Cr below neutral", V < 128);
		});

		It("should keep every sample inside the legal range", [this]()
		{
			// Sweep the extremes that would overflow a naive implementation
			const uint8 Corners[][3] = {
				{ 0, 0, 0 }, { 255, 255, 255 }, { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 0, 255 }
			};

			for (const uint8* Corner : Corners)
			{
				uint8 Y = 0, U = 0, V = 0;
				ConvertFlat(Corner[0], Corner[1], Corner[2], 32, 32, Y, U, V);

				TestTrue("luma >= 16", Y >= 16);
				TestTrue("luma <= 235", Y <= 235);
				TestTrue("Cb in range", U >= 16 && U <= 240);
				TestTrue("Cr in range", V >= 16 && V <= 240);
			}
		});

		It("should handle odd dimensions without reading out of bounds", [this]()
		{
			uint8 Y = 0, U = 0, V = 0;
			ConvertFlat(10, 200, 30, 33, 17, Y, U, V);

			TestTrue("luma in range", Y >= 16 && Y <= 235);
			TestTrue("Cb in range", U >= 16 && U <= 240);
			TestTrue("Cr in range", V >= 16 && V <= 240);
		});
	});

	Describe("Scaled BGRA to I420", [this]()
	{
		It("should preserve aspect ratio and force even dimensions", [this]()
		{
			uint32 W = 0, H = 0;
			FSentryColorConversion::ComputeScaledSize(3840, 2160, 720, W, H);
			TestEqual("height", static_cast<int32>(H), 720);
			TestEqual("width", static_cast<int32>(W), 1280);

			// 1828x1142 is the odd-sized window the desktop capture actually produced
			FSentryColorConversion::ComputeScaledSize(1828, 1142, 720, W, H);
			TestEqual("height", static_cast<int32>(H), 720);
			TestTrue("width is even", (W % 2) == 0);
			TestTrue("aspect preserved", FMath::Abs(static_cast<int32>(W) - 1152) <= 2);
		});

		It("should leave the size alone when already within the limit", [this]()
		{
			uint32 W = 0, H = 0;
			FSentryColorConversion::ComputeScaledSize(1280, 720, 720, W, H);
			TestEqual("width", static_cast<int32>(W), 1280);
			TestEqual("height", static_cast<int32>(H), 720);

			FSentryColorConversion::ComputeScaledSize(3840, 2160, 0, W, H);
			TestEqual("width", static_cast<int32>(W), 3840);
			TestEqual("height", static_cast<int32>(H), 2160);
		});

		It("should round odd source dimensions down to even", [this]()
		{
			uint32 W = 0, H = 0;
			FSentryColorConversion::ComputeScaledSize(1921, 1081, 0, W, H);
			TestEqual("width", static_cast<int32>(W), 1920);
			TestEqual("height", static_cast<int32>(H), 1080);
		});

		It("should match the unscaled path at identity scale", [this]()
		{
			TArray<uint8> Scaled;
			int32 Stride = 0;
			ConvertSplit(0, 0, 0, 255, 255, 255, 64, 64, 32, 64, 64, Scaled, Stride);

			TestEqual("left luma", static_cast<int32>(Scaled[Stride * 32 + 8]), 16);
			TestEqual("right luma", static_cast<int32>(Scaled[Stride * 32 + 56]), 235);
		});

		It("should keep the split in place after a 4x downscale", [this]()
		{
			TArray<uint8> Scaled;
			int32 Stride = 0;
			ConvertSplit(0, 0, 0, 255, 255, 255, 256, 256, 128, 64, 64, Scaled, Stride);

			// Sampled well inside each half, so the boundary column is not involved
			TestEqual("left luma", static_cast<int32>(Scaled[Stride * 32 + 8]), 16);
			TestEqual("right luma", static_cast<int32>(Scaled[Stride * 32 + 56]), 235);
		});

		It("should average across the boundary rather than dropping pixels", [this]()
		{
			TArray<uint8> Scaled;
			int32 Stride = 0;
			// At a 2:1 scale a split on an even column always lands on a box
			// boundary, so put it on an odd one: destination column 10 then covers
			// source columns 20 and 21, one from each half
			ConvertSplit(0, 0, 0, 255, 255, 255, 64, 64, 21, 32, 32, Scaled, Stride);

			// A box filter must land between the extremes here; nearest-neighbour
			// would snap to one end
			const int32 Boundary = static_cast<int32>(Scaled[Stride * 16 + 10]);
			TestTrue("boundary is averaged", Boundary > 16 && Boundary < 235);
		});

		It("should refuse to scale up rather than read out of bounds", [this]()
		{
			TArray<uint8> Y, U, V, Src;
			Y.SetNumZeroed(64 * 64);
			U.SetNumZeroed(32 * 32);
			V.SetNumZeroed(32 * 32);
			Src.SetNumZeroed(32 * 32 * 4);

			FSentryColorConversion::BgraToI420Scaled(Src.GetData(), 32 * 4, 32, 32, 64, 64,
				Y.GetData(), U.GetData(), V.GetData(), 64, 32);

			TestEqual("output untouched", static_cast<int32>(Y[0]), 0);
		});
	});
}

#endif // WITH_AUTOMATION_TESTS && USE_SENTRY_SESSION_REPLAY
