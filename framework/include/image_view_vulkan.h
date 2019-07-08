#pragma once

namespace cgb
{
	/** Class representing an image view. */
	class image_view_t
	{
	public:
		image_view_t() = default;
		image_view_t(const image_view_t&) = delete;
		image_view_t(image_view_t&&) = default;
		image_view_t& operator=(const image_view_t&) = delete;
		image_view_t& operator=(image_view_t&&) = default;
		~image_view_t() = default;

		/** Get the config which is used to created this image view with the API. */
		const auto& config() const { return mInfo; }
		/** Get the config which is used to created this image view with the API. */
		auto& config() { return mInfo; }
		/** Gets the image handle. */
		const auto& image_handle() const { return cgb::get(mImage).image_handle(); }
		/** Gets the image instance which this view has been created for. */
		const auto& image() const { return cgb::get(mImage); }

		/** Creates a new image view upon a given image
		*	@param	pImageToOwn					The image which to create an image view for
		*	@param	pViewFormat					The format of the image view
		*	@param	pAlterConfigBeforeCreation	A context-specific function which allows to modify the `vk::ImageViewCreateInfo` just before the image view will be created. Use `.config()` to access the configuration structure!
		*	@return	Returns a newly created image.
		*/
		static image_view_t create(cgb::image pImageToOwn, image_format pViewFormat, context_specific_function<void(image_view_t&)> pAlterConfigBeforeCreation = {});

	private:
		// OWNING handle to an image. (Make sure it gets destructed after the image view)
		cgb::image mImage;
		// Config which is passed to the create call and contains all the parameters for image view creation.
		vk::ImageViewCreateInfo mInfo;
		// The image view's handle. This member will contain a valid handle only after successful image view creation.
		vk::UniqueImageView mImageView;
	};

	/** Typedef representing any kind of OWNING image view representations. */
	using image_view = std::variant<image_view_t, std::unique_ptr<image_view_t>, std::shared_ptr<image_view_t>>;
}
