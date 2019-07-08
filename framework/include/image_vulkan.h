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

	/** Represents an image and its associated memory
	 */
	class image_t
	{
	public:
		image_t() = default;
		image_t(const image_t&) = delete;
		image_t(image_t&&) = default;
		image_t& operator=(const image_t&) = delete;
		image_t& operator=(image_t&&) = default;
		~image_t() = default;

		/** Get the config which is used to created this image with the API. */
		const auto& config() const { return mInfo; }
		/** Get the config which is used to created this image with the API. */
		auto& config() { return mInfo; }
		/** Gets the image handle. */
		const auto& image_handle() const { return mImage.get(); }
		/** Gets the handle to the image's memory. */
		const auto& memory_handle() const { return mMemory.get(); }

		/** Creates a new image
		 *	@param	pWidth						The width of the image to be created
		 *	@param	pHeight						The height of the image to be created
		 *	@param	pFormat						The image format of the image to be created
		 *	@param	pMemoryUsage				Where the memory of the image shall be allocated (GPU or CPU) and how it is going to be used.
		 *	@param	pUseMipMaps					Whether or not MIP maps shall be created for this image. Specifying `true` will set the maximum number of MIP map images.
		 *	@param	pNumLayers					How many layers the image to be created shall contain.
		 *	@param	pAlterConfigBeforeCreation	A context-specific function which allows to modify the `vk::ImageCreateInfo` just before the image will be created. Use `.config()` to access the configuration structure!
		 *	@return	Returns a newly created image.
		 */
		static image_t create(int pWidth, int pHeight, image_format pFormat, memory_usage pMemoryUsage, bool pUseMipMaps = false, int pNumLayers = 1, context_specific_function<void(image_t&)> pAlterConfigBeforeCreation = {});

		// TODO: What to do with this one: ??
		vk::ImageMemoryBarrier create_barrier(vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange = std::nullopt) const;

	private:
		// The image create info which contains all the parameters for image creation
		vk::ImageCreateInfo mInfo;
		// The image handle. This member will contain a valid handle only after successful image creation.
		vk::UniqueImage mImage;
		// The memory handle. This member will contain a valid handle only after successful image creation.
		vk::UniqueDeviceMemory mMemory;
	};

	/** Typedef representing any kind of OWNING image representations. */
	using image	= std::variant<image_t, std::unique_ptr<image_t>, std::shared_ptr<image_t>>;

}
