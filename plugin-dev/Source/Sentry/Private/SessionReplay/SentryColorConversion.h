// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

/**
 * Pixel-format conversion for the software encoder path.
 *
 * Hardware encoders take the captured BGRA texture as a GPU resource and do the
 * colour conversion themselves. openh264 wants planar YUV in system memory, so
 * the software path converts on the CPU after reading the texture back.
 */
class FSentryColorConversion
{
public:
	/**
	 * Converts a BGRA8 image to planar I420 (YUV 4:2:0), BT.709 limited range.
	 *
	 * The output must be signalled to match - see FSentryOpenH264Encoder, which
	 * sets the SPS VUI to BT.709 with bFullRange = false. Mismatching the two is
	 * the classic silent failure here: the replay still plays, it just looks
	 * washed out or crushed.
	 *
	 * Chroma is produced by averaging each 2x2 block in RGB before converting,
	 * which is both cheaper and slightly better than converting then averaging.
	 * Odd dimensions are handled by clamping the sample position, so the last
	 * row/column is duplicated rather than read out of bounds.
	 *
	 * @param Bgra       source pixels, B at byte 0, G at 1, R at 2, A at 3
	 * @param SrcStride  source bytes per row; may exceed Width * 4
	 * @param Width      image width in pixels
	 * @param Height     image height in pixels
	 * @param OutY       luma plane, at least YStride * Height bytes
	 * @param OutU       Cb plane, at least CStride * ((Height + 1) / 2) bytes
	 * @param OutV       Cr plane, same size as OutU
	 * @param YStride    bytes per row in OutY; must be >= Width
	 * @param CStride    bytes per row in OutU/OutV; must be >= (Width + 1) / 2
	 */
	static void BgraToI420(const uint8* Bgra, int32 SrcStride, uint32 Width, uint32 Height,
		uint8* OutY, uint8* OutU, uint8* OutV, int32 YStride, int32 CStride);

	/**
	 * As BgraToI420, but box-filters the source down to DstWidth x DstHeight on the way.
	 *
	 * Encoding cost and clip size both scale with pixel count, and a 4K backbuffer
	 * is roughly nine times the work of 720p for a recording nobody inspects
	 * pixel-by-pixel. Scaling here rather than on the GPU keeps the capture path
	 * free of shaders and RHI-version differences; the readback still moves the
	 * full frame, but the encode - which dominates - drops with the pixel count.
	 *
	 * Dst dimensions must be no larger than the source, and are rounded to even
	 * numbers by the caller so chroma subsampling stays exact.
	 */
	static void BgraToI420Scaled(const uint8* Bgra, int32 SrcStride, uint32 SrcWidth, uint32 SrcHeight,
		uint32 DstWidth, uint32 DstHeight,
		uint8* OutY, uint8* OutU, uint8* OutV, int32 YStride, int32 CStride);

	// Largest even-dimensioned size within MaxHeight that preserves aspect ratio.
	// Returns the source size unchanged when MaxHeight is 0 or already large enough
	static void ComputeScaledSize(uint32 SrcWidth, uint32 SrcHeight, int32 MaxHeight,
		uint32& OutWidth, uint32& OutHeight);
};

#endif // USE_SENTRY_SESSION_REPLAY
