#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
	struct depth_test
	{
		static depth_test enabled() { return depth_test{ true }; }
		static depth_test disabled() { return depth_test{ false }; }
		bool mEnabled;
	};

	struct depth_write
	{
		static depth_write enabled() { return depth_write{ true }; }
		static depth_write disabled() { return depth_write{ false }; }
		bool mEnabled;
	};




	/** Represents data for a vulkan graphics pipeline 
	*	The data held by such a struct is a triple of:
	*    - render pass
	*    - pipeline layout
	*    - pipeline handle
	*/
	class pipeline
	{
	public:
		pipeline() = default;
		pipeline(pipeline&&) = default;
		pipeline(const pipeline&) = delete;
		pipeline& operator=(pipeline&&) = default;
		pipeline& operator=(const pipeline&) = delete;
		~pipeline() = default;


		static pipeline prepare(
			std::vector<shader> pShaders,
			std::vector<descriptor_set_layout> pDescriptorSetLayouts, 
			depth_test pDepthTestConfig, 
			depth_write pDepthWriteConfig);

		void build();

	private:
		std::vector<shader> mShaders;
		vk::PipelineDepthStencilStateCreateInfo mDepthStencilConfig;
		//vk::PipelineLayoutCreateInfo

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
