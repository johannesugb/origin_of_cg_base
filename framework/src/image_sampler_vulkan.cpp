#include "image_sampler_vulkan.h"

namespace cgb
{
	image_sampler_t image_sampler_t::create(image pImage, sampler pSampler)
	{
		image_sampler_t result;
		result.mImage = std::move(pImage);
		result.mSampler = std::move(pSampler);
	}
}
