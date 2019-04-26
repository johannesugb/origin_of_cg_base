#pragma once

namespace cgb
{
	/** Dimensions of a window */
	struct window_size
	{
		window_size(int pWidth, int pHeight)
			: mWidth{ pWidth }
			, mHeight{ pHeight }
		{}

		int mWidth;
		int mHeight;
	};

	/** The order of the color channels w.r.t. an image format */
	enum struct image_color_channel_order
	{
		rgba,
		bgra,
		argb,
		abgr,
		rgb = rgba,
		bgr = bgra
	};

	/** The format of a color channel */
	enum struct image_color_channel_format
	{
		uint8,
		uint8_srgb,
		int8,
		uint16,
		int16,
		uint32,
		int32,
		uint64,
		int64,
		float16,
		float32,
		float64
	};
}
