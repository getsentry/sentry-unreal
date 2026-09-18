// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#if defined(WITH_AUTOMATION_TESTS) && defined(USE_SENTRY_SESSION_REPLAY)

#include "SessionReplay/SentryColorConversion.h"

BEGIN_DEFINE_SPEC(SentryColorConversionSpec, "Sentry.SessionReplay.ColorConversion", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)

	// Fills a BGRA image with one flat colour and converts it, returning the
	// centre sample of each plane.
	void ConvertFlat(uint8 B, uint8 G, uint8 R, uint32 Width, uint32 Height, uint8 & OutY, uint8 & OutU, uint8 & OutV);

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
}

#endif // WITH_AUTOMATION_TESTS && USE_SENTRY_SESSION_REPLAY
