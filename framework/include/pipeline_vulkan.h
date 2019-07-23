#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
	/** Represents data for a vulkan graphics pipeline 
	*	The data held by such a struct is a triple of:
	*    - render pass
	*    - pipeline layout
	*    - pipeline handle
	*/
	class graphics_pipeline
	{
	public:
		graphics_pipeline() = default;
		graphics_pipeline(graphics_pipeline&&) = default;
		graphics_pipeline(const graphics_pipeline&) = delete;
		graphics_pipeline& operator=(graphics_pipeline&&) = default;
		graphics_pipeline& operator=(const graphics_pipeline&) = delete;
		~graphics_pipeline() = default;

		const auto& handle() const { return mPipeline; }

		static graphics_pipeline create(const graphics_pipeline_config& _Config, cgb::context_specific_function<void(graphics_pipeline&)> _AlterConfigBeforeCreation = {});

	private:
		std::vector<vk::VertexInputBindingDescription> mVertexInputBindingDescriptions;
		vk::PipelineVertexInputStateCreateInfo mPipelineVertexInputStateCreateInfo;

		std::vector<shader_info> mShaderInfos;
		depth_test mDepthTestConfig;
		depth_write mDepthWriteConfig;
		std::vector<viewport_depth_scissors_config> mViewportDepthConfig;
		culling_mode mCullingMode;
		front_face mFrontFaceWindingOrder;
		polygon_drawing mPolygonDrawingModeAndConfig;
		rasterizer_geometry_mode mRasterizerGeometryMode;
		depth_settings mDepthSettings;
		color_blending_settings mColorBlendingSettings;
		std::vector<color_blending_config> mColorBlendingPerAttachment;
		std::vector<binding_data> mResourceBindings;
		std::vector<push_constant_binding_data> mPushConstantsBindings;


		std::vector<shader> mShaders;
		vk::PipelineDepthStencilStateCreateInfo mDepthStencilConfig;
		//vk::PipelineLayoutCreateInfo
		vk::PipelineCreateFlagBits mPipelineCreateFlags;

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
