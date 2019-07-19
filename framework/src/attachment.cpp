namespace cgb
{
	attachment attachment::create_color(image_format pFormat, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is a depth format, but is used for a color attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
			false,
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
			true,
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
			is_depth_format(pFormat),
			1,		// num samples
			false,	// resolve
			true	// shader input
		};
	}

	attachment attachment::create_color_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation)
	{
#if defined(_DEBUG)
		if (!is_depth_format(pFormat)) {
			LOG_WARNING("The specified image_format is a depth format, but is used for a color attachment.");
		}
#endif
		return attachment{
			pLocation,
			pFormat,
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
			true,
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
			false,
			pSampleCount,			// num samples
			pResolveMultisamples,	// resolve
			true					// shader input
		};
	}

}