// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryColorConversion.h"

#ifdef USE_SENTRY_SESSION_REPLAY

namespace SentryColorConversionDetail
{
// BT.709 limited range, RGB [0,255] -> Y [16,235] / CbCr [16,240].
// Coefficients are Q16 fixed point:
//   Y =  0.1826R + 0.6142G + 0.0620B + 16
//   U = -0.1006R - 0.3386G + 0.4392B + 128
//   V =  0.4392R - 0.3989G - 0.0403B + 128
constexpr int32 Yr = 11966;
constexpr int32 Yg = 40254;
constexpr int32 Yb = 4064;

constexpr int32 Ur = -6595;
constexpr int32 Ug = -22190;
constexpr int32 Ub = 28782;

constexpr int32 Vr = 28782;
constexpr int32 Vg = -26145;
constexpr int32 Vb = -2637;

constexpr int32 Q = 16;
constexpr int32 Half = 1 << (Q - 1);
constexpr int32 LumaOffset = (16 << Q) + Half;
constexpr int32 ChromaOffset = (128 << Q) + Half;

FORCEINLINE uint8 ClampByte(int32 Value)
{
	return static_cast<uint8>(Value < 0 ? 0 : (Value > 255 ? 255 : Value));
}
} // namespace SentryColorConversionDetail

void FSentryColorConversion::BgraToI420(const uint8* Bgra, int32 SrcStride, uint32 Width, uint32 Height,
	uint8* OutY, uint8* OutU, uint8* OutV, int32 YStride, int32 CStride)
{
	using namespace SentryColorConversionDetail;

	if (Bgra == nullptr || OutY == nullptr || OutU == nullptr || OutV == nullptr)
	{
		return;
	}
	if (Width == 0 || Height == 0 || SrcStride < static_cast<int32>(Width) * 4)
	{
		return;
	}
	if (YStride < static_cast<int32>(Width) || CStride < static_cast<int32>((Width + 1) / 2))
	{
		return;
	}

	// Luma: one sample per pixel.
	for (uint32 Row = 0; Row < Height; ++Row)
	{
		const uint8* SrcRow = Bgra + static_cast<int64>(Row) * SrcStride;
		uint8* DstRow = OutY + static_cast<int64>(Row) * YStride;

		for (uint32 Col = 0; Col < Width; ++Col)
		{
			const int32 B = SrcRow[Col * 4 + 0];
			const int32 G = SrcRow[Col * 4 + 1];
			const int32 R = SrcRow[Col * 4 + 2];

			DstRow[Col] = ClampByte((R * Yr + G * Yg + B * Yb + LumaOffset) >> Q);
		}
	}

	// Chroma: average each 2x2 block in RGB, then convert once.
	const uint32 ChromaWidth = (Width + 1) / 2;
	const uint32 ChromaHeight = (Height + 1) / 2;

	for (uint32 CRow = 0; CRow < ChromaHeight; ++CRow)
	{
		const uint32 Row0 = CRow * 2;
		const uint32 Row1 = FMath::Min(Row0 + 1, Height - 1);

		const uint8* SrcRow0 = Bgra + static_cast<int64>(Row0) * SrcStride;
		const uint8* SrcRow1 = Bgra + static_cast<int64>(Row1) * SrcStride;

		uint8* DstU = OutU + static_cast<int64>(CRow) * CStride;
		uint8* DstV = OutV + static_cast<int64>(CRow) * CStride;

		for (uint32 CCol = 0; CCol < ChromaWidth; ++CCol)
		{
			const uint32 Col0 = CCol * 2;
			const uint32 Col1 = FMath::Min(Col0 + 1, Width - 1);

			const int32 B = SrcRow0[Col0 * 4 + 0] + SrcRow0[Col1 * 4 + 0] +
							SrcRow1[Col0 * 4 + 0] + SrcRow1[Col1 * 4 + 0];
			const int32 G = SrcRow0[Col0 * 4 + 1] + SrcRow0[Col1 * 4 + 1] +
							SrcRow1[Col0 * 4 + 1] + SrcRow1[Col1 * 4 + 1];
			const int32 R = SrcRow0[Col0 * 4 + 2] + SrcRow0[Col1 * 4 + 2] +
							SrcRow1[Col0 * 4 + 2] + SrcRow1[Col1 * 4 + 2];

			// R/G/B here are 4x the average, so divide the Q16 result by 4 as well
			DstU[CCol] = ClampByte(((R * Ur + G * Ug + B * Ub) / 4 + ChromaOffset) >> Q);
			DstV[CCol] = ClampByte(((R * Vr + G * Vg + B * Vb) / 4 + ChromaOffset) >> Q);
		}
	}
}

#endif // USE_SENTRY_SESSION_REPLAY
