#pragma once

namespace cgb
{
	/** Represents one specific native image format for the Vulkan context */
	struct image_format
	{
		image_format() noexcept;
		image_format(const vk::Format& pFormat) noexcept;
		image_format(const vk::SurfaceFormatKHR& pSrfFrmt) noexcept;

		vk::Format mFormat;
	};

	class image_t ///////////////// TODO: Check das nochmals und auch die create method
	{
	public:
		image_t() = default;
		image_t(const image_t&) = delete;
		image_t(image_t&&) = default;
		image_t& operator=(const image_t&) = delete;
		image_t& operator=(image_t&&) = default;
		~image_t() = default;

		const vk::ImageCreateInfo& create_info() const { return mInfo; }
		vk::ImageCreateInfo& create_info() { return mInfo; }
		const vk::Image& image() const { return mImage.get(); }
		const vk::DeviceMemory& memory() const { return mMemory.get(); }

		static image_t create(int pWidth, int pHeight, image_format pFormat, memory_usage pMemoryUsage, bool pUseMipMaps = false, int pNumLayers = 1, context_specific_function<void(image_t&)> pAlterConfigBeforeCreation = {});

		// TODO: What to do with this one: ??
		vk::ImageMemoryBarrier create_barrier(vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange = std::nullopt) const;

	private:
		vk::ImageCreateInfo mInfo;
		vk::UniqueImage mImage;
		vk::UniqueDeviceMemory mMemory;
	};

	using image	= std::variant<image_t, std::unique_ptr<image_t>, std::shared_ptr<image_t>>;

}
