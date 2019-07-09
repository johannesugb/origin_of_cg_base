#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
	/** Represents data for a vulkan graphics pipeline 
	*	The data held by such a struct is a triple of:
	*    - render pass
	*    - pipeline layout
	*    - pipeline handle
	*/
	struct pipeline
	{
		pipeline() noexcept;
		pipeline(const vk::PipelineLayout& pPipelineLayout, const vk::Pipeline& pPipeline, vk::RenderPass pRenderPass = nullptr) noexcept;
		pipeline(const pipeline&) = delete;
		pipeline(pipeline&&) noexcept;
		pipeline& operator=(const pipeline&) = delete;
		pipeline& operator=(pipeline&&) noexcept;
		~pipeline();

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
