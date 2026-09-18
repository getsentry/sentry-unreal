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
};

#endif // USE_SENTRY_SESSION_REPLAY
