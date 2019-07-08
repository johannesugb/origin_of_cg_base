#pragma once

namespace cgb
{
	class image_sampler_t
	{
	public:
		image_sampler_t() = default;
		image_sampler_t(const image_sampler_t&) = delete;
		image_sampler_t(image_sampler_t&&) = default;
		image_sampler_t& operator=(const image_sampler_t&) = delete;
		image_sampler_t& operator=(image_sampler_t&&) = default;
		~image_sampler_t() = default;

		static image_sampler_t create(image pImage, sampler pSampler);

	private:
		image mImage;
		sampler mSampler;
	};

	/** Typedef representing any kind of OWNING image-sampler representations. */
	using image_sampler = std::variant<image_sampler_t, std::unique_ptr<image_sampler_t>, std::shared_ptr<image_sampler_t>>;
}
