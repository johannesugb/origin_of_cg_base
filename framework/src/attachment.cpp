namespace cgb
{
	attachment attachment::create_color(image_format pFormat, bool pIsPresentable, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is a depth format, but is used for a color attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
			pIsPresentable,
			false,	// is not depth
			1,		// num samples
			false,	// resolve
			false	// shader input
		};
	}

	attachment attachment::create_depth(image_format pFormat, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is probably not a depth format, but is used for a depth attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
			false,	// Assume that this shall not be presented
			true,	// is depth
			1,		// num samples
			false,	// resolve
			false	// shader input
		};
	}

	attachment attachment::create_shader_input(image_format pFormat, std::optional<uint32_t> pLocation)
	{
		return attachment{
			pLocation,
			pFormat,
			false,	// Assume that this shall not be presented
			is_depth_format(pFormat), // might be depth
			1,		// num samples
			false,	// resolve
			true	// shader input
		};
	}

	attachment attachment::create_color_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, bool pIsPresentable, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is a depth format, but is used for a color attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
			pIsPresentable,
			false,
			pSampleCount,			// num samples
			pResolveMultisamples,	// resolve
			false					// shader input
		};
	}

	attachment attachment::create_depth_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is probably not a depth format, but is used for a depth attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
			false,					// Assume that this shall not be presented
			true,					// is depth
			pSampleCount,			// num samples
			pResolveMultisamples,	// resolve
			false					// shader input
		};
	}

	attachment attachment::create_shader_input_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation)
	{
		return attachment{
			pLocation,
			pFormat,
			false,					// Assume that this shall not be presented
			false,					// is not depth
			pSampleCount,			// num samples
			pResolveMultisamples,	// resolve
			true					// shader input
		};
	}

}