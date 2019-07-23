namespace cgb
{
	using namespace cpplinq;

	graphics_pipeline graphics_pipeline::create(const graphics_pipeline_config& _Config, cgb::context_specific_function<void(graphics_pipeline&)> _AlterConfigBeforeCreation)
	{
		graphics_pipeline result;

		{
			// Select DISTINCT bindings:
			auto bindings = from(_Config.mInputBindingLocations)
				>> select([](const input_binding_location_data& _BindingData) { return _BindingData.mGeneralData; })
				>> distinct()
				>> orderby([](const input_binding_general_data& _GeneralData) { return _GeneralData.mBinding; })
				>> to_vector();
			result.mVertexInputBindingDescriptions.reserve(bindings.size());

			for (auto& bindingData : bindings) {

				auto numRecordsWithSameBinding = std::count_if(std::begin(bindings), std::end(bindings), 
					[bindingId = bindingData.mBinding](const input_binding_general_data& _GeneralData) {
						return _GeneralData.mBinding == bindingId;
					});
				if (1 != numRecordsWithSameBinding) {
					throw std::runtime_error(fmt::format("The input binding {} is defined in different ways. Make sure to define it uniformly across different bindings!", bindingData.mBinding));
				}

				result.mVertexInputBindingDescriptions.push_back(vk::VertexInputBindingDescription()
					// The following parameters are guaranteed to be the same. We have checked this.
					.setBinding(bindingData.mBinding)
					.setStride(bindingData.mStride)
					.setInputRate(to_vk_vertex_input_rate(bindingData.mKind))
					// Don't need the location here
				);
			}
		}

		// TODO: Next, define the vertex attribute descriptions!
		result. mPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(static_cast<uint32_t>(result.mVertexInputBindingDescriptions.size()))
			.setPVertexBindingDescriptions(result.mVertexInputBindingDescriptions.data())
			.setvertexattrib


		// Maybe alter the config?!
		if (_AlterConfigBeforeCreation.mFunction) {
			_AlterConfigBeforeCreation.mFunction(result);
		}

		// PIPELINE CREATION, a.k.a. putting it all together:
		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputinfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportInfo)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisamplingInfo)
			.setPDepthStencilState(&mDepthStencilConfig) // Optional
			.setPColorBlendState(&colorBlendingInfo)
			.setPDynamicState(nullptr) // Optional
			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0u)
			.setBasePipelineHandle(nullptr) // Optional
			.setBasePipelineIndex(-1); // Optional

		context().logical_device().createGraphicsPipelineUnique()
		return result;
	}

	graphics_pipeline graphics_pipeline::prepare(
		std::vector<shader> _Shaders,
		std::vector<descriptor_set_layout> _DescriptorSetLayouts, 
		depth_test _DepthTestConfig, 
		depth_write _DepthWriteConfig)
	{
		graphics_pipeline result;
		result.mShaders = std::move(pShaders);

		result.mDepthStencilConfig
			.setDepthTestEnable(to_vk_bool(_DepthTestConfig.mEnabled))
			.setDepthWriteEnable(to_vk_bool(_DepthWriteConfig.mEnabled))
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(VK_FALSE)
			.setFront(vk::StencilOpState())
			.setBack(vk::StencilOpState());

		return result;
	}

	void graphics_pipeline::build()
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


	}
}
