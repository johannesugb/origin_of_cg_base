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

		const auto& get_image() const			{ return cgb::get(mImageView); }
		const auto& get_sampler() const			{ return cgb::get(mSampler); }
		const auto& view_handle() const			{ return cgb::get(mImageView).view_handle(); }
		const auto& image_handle() const		{ return cgb::get(mImageView).image_handle(); }
		const auto& sampler_handle() const		{ return cgb::get(mSampler).handle(); }
		const auto& descriptor_info() const		{ return mDescriptorInfo; }
		const auto& descriptor_type() const		{ return mDescriptorType; }

		static image_sampler_t create(image_view pImageView, sampler pSampler);

	private:
		image_view mImageView;
		sampler mSampler;
		vk::DescriptorImageInfo mDescriptorInfo;
		vk::DescriptorType mDescriptorType;
	};

	/** Typedef representing any kind of OWNING image-sampler representations. */
	using image_sampler = std::variant<image_sampler_t, std::unique_ptr<image_sampler_t>, std::shared_ptr<image_sampler_t>>;
}
