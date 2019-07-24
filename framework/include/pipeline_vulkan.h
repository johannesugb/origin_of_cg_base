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
		// The vertex input data:
		std::vector<vk::VertexInputBindingDescription> mVertexInputBindingDescriptions;
		std::vector<vk::VertexInputAttributeDescription> mVertexInputAttributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo mPipelineVertexInputStateCreateInfo;
		// How to interpret the vertex input:
		vk::PipelineInputAssemblyStateCreateInfo mInputAssemblyStateCreateInfo;
		// Our precious GPU shader programs:
		std::vector<shader> mShaders;
		std::vector<vk::PipelineShaderStageCreateInfo> mShaderStageCreateInfos;
		// Viewport, depth, and scissors configuration
		std::vector<vk::Viewport> mViewports;
		std::vector<vk::Rect2D> mScissors;
		vk::PipelineViewportStateCreateInfo mViewportStateCreateInfo;
		// Rasterization state:
		vk::PipelineRasterizationStateCreateInfo mRasterizationStateCreateInfo;
		// TODO: Proceed here


		vk::PipelineDepthStencilStateCreateInfo mDepthStencilConfig;
		//vk::PipelineLayoutCreateInfo
		vk::PipelineCreateFlagBits mPipelineCreateFlags;

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
