#pragma once

namespace cgb
{
	/** Describes an attachment to a framebuffer or a renderpass.
	 *	It can describe color attachments as well as depth/stencil attachments
	 *	and holds some additional config parameters for these attachments.
	 */
	struct attachment
	{
		/** Create a new color attachment
		 *	@param	pFormat		The color format of the new attachment
		 *	@param	pLocation	(Optional) At which layout location shall this color attachment appear.
		 */
		static attachment create_color(image_format pFormat, std::optional<uint32_t> pLocation = {});

		/** Create a new depth/stencil attachment
		 *	@param	pFormat		The depth/stencil format of the new attachment
		 *	@param	pLocation	(Optional) At which layout location shall this depth/stencil attachment appear.
		 */
		static attachment create_depth(image_format pFormat, std::optional<uint32_t> pLocation = {});

		/** Create a new color attachment for use as shader input
		 *	@param	pFormat		The color format of the new attachment
		 *	@param	pLocation	(Optional) At which layout location shall this color attachment appear.
		 */
		static attachment create_shader_input(image_format pFormat, std::optional<uint32_t> pLocation = {});

		/** Create a new multisampled color attachment
		 *	@param	pFormat					The color format of the new attachment
		 *	@param	pSampleCount			The number of samples per fragment. A value of 1 means that multisampling is disabled.
		 *	@param	pResolveMultisamples	If set to true, a multisample resolve pass will be set up.
		 *	@param	pLocation				(Optional) At which layout location shall this color attachment appear.
		 */
		static attachment create_color_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation = {});

		/** Create a new multisampled depth/stencil attachment
		 *	@param	pFormat					The depth/stencil format of the new attachment
		 *	@param	pSampleCount			The number of samples per fragment. A value of 1 means that multisampling is disabled.
		 *	@param	pResolveMultisamples	If set to true, a multisample resolve pass will be set up.
		 *	@param	pLocation				(Optional) At which layout location shall this depth/stencil attachment appear.
		 */
		static attachment create_depth_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation = {});

		/** Create a new multisampled color attachment for use as shader input
		 *	@param	pFormat					The color format of the new attachment
		 *	@param	pSampleCount			The number of samples per fragment. A value of 1 means that multisampling is disabled.
		 *	@param	pResolveMultisamples	If set to true, a multisample resolve pass will be set up.
		 *	@param	pLocation				(Optional) At which layout location shall this color attachment appear.
		 */
		static attachment create_shader_input_multisampled(image_format pFormat, int pSampleCount, bool pResolveMultisamples, std::optional<uint32_t> pLocation = {});

		/** The color/depth/stencil format of the attachment */
		auto format() const { return mFormat; }
		/** True if a specific location has been configured */
		bool has_specific_location() const { return mLocation.has_value(); }
		/** The location or 0 if no specific location has been configured */
		uint32_t location() const { return mLocation.value_or(0u); }
		/** True if this attachment represents not a depth/stencil attachment, but instead, a color attachment */
		auto is_color_attachment() const { return !mIsDepthAttachment; }
		/** True if this attachment represents a depth/stencil attachment. */
		auto is_depth_attachment() const { return mIsDepthAttachment; }
		/** True fi the sample count is greater than 1 */
		bool is_multisampled() const { return mSampleCount > 1; }
		/** The sample count for this attachment. */
		auto sample_count() const { return mSampleCount; }
		/** True if a multisample resolve pass shall be set up. */
		auto resolve_samples() const { return mDoResolve; }
		/** True if this attachment is intended to be used as input attachment to a shader. */
		auto is_shader_input_attachment() const { return mIsShaderInputAttachment; }

		std::optional<uint32_t> mLocation;
		image_format mFormat;
		bool mIsDepthAttachment;
		int mSampleCount;
		bool mDoResolve;
		bool mIsShaderInputAttachment;
	};
}
