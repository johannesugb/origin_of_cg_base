namespace cgb
{
	pipeline pipeline::prepare(
		std::vector<shader> pShaders,
		std::vector<descriptor_set_layout> pDescriptorSetLayouts, 
		depth_test pDepthTestConfig, 
		depth_write pDepthWriteConfig)
	{
		pipeline result;
		result.mShaders = std::move(pShaders);

		result.mDepthStencilConfig
			.setDepthTestEnable(to_vk_bool(pDepthTestConfig.mEnabled))
			.setDepthWriteEnable(to_vk_bool(pDepthWriteConfig.mEnabled))
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(VK_FALSE)
			.setFront(vk::StencilOpState())
			.setBack(vk::StencilOpState());
	}

	void pipeline::build()
	{
		// Shader Stages:
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(mShaders.size());
		for (auto& s : mShaders) {
			assert(s.has_been_built());
			shaderStages.push_back(vk::PipelineShaderStageCreateInfo()
				.setStage(to_vk_shader_stage(s.info().mShaderType))
				.setModule(s.handle())
				.setPName(s.info().mEntryPoint.c_str()));
		}

		//// PIPELINE CREATION, a.k.a. putting it all together:
		//auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		//	.setStageCount(static_cast<uint32_t>(shaderStages.size()))
		//	.setPStages(shaderStages.data())
		//	.setPVertexInputState(&vertexInputinfo)
		//	.setPInputAssemblyState(&inputAssembly)
		//	.setPViewportState(&viewportInfo)
		//	.setPRasterizationState(&rasterizer)
		//	.setPMultisampleState(&multisamplingInfo)
		//	.setPDepthStencilState(&mDepthStencilConfig) // Optional
		//	.setPColorBlendState(&colorBlendingInfo)
		//	.setPDynamicState(nullptr) // Optional
		//	.setLayout(pipelineLayout)
		//	.setRenderPass(renderPass)
		//	.setSubpass(0u)
		//	.setBasePipelineHandle(nullptr) // Optional
		//	.setBasePipelineIndex(-1); // Optional
	}
}
