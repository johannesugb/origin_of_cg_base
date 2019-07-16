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

		static graphics_pipeline create(const graphics_pipeline_config& pConfig);

	private:
		std::vector<shader> mShaders;
		vk::PipelineDepthStencilStateCreateInfo mDepthStencilConfig;
		//vk::PipelineLayoutCreateInfo
		vk::PipelineCreateFlagBits mPipelineCreateFlags;

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
