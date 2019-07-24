namespace cgb
{
	using namespace cpplinq;

	graphics_pipeline graphics_pipeline::create(const graphics_pipeline_config& _Config, cgb::context_specific_function<void(graphics_pipeline&)> _AlterConfigBeforeCreation)
	{
		graphics_pipeline result;

		// 1. Compile the array of vertex input binding descriptions
		{ 
			// Select DISTINCT bindings:
			auto bindings = from(_Config.mInputBindingLocations)
				>> select([](const input_binding_location_data& _BindingData) { return _BindingData.mGeneralData; })
				>> distinct() // see what I did there
				>> orderby([](const input_binding_general_data& _GeneralData) { return _GeneralData.mBinding; })
				>> to_vector();
			result.mVertexInputBindingDescriptions.reserve(bindings.size());

			for (auto& bindingData : bindings) {

				const auto numRecordsWithSameBinding = std::count_if(std::begin(bindings), std::end(bindings), 
					[bindingId = bindingData.mBinding](const input_binding_general_data& _GeneralData) {
						return _GeneralData.mBinding == bindingId;
					});
				if (1 != numRecordsWithSameBinding) {
					throw std::runtime_error(fmt::format("The input binding {} is defined in different ways. Make sure to define it uniformly across different bindings/attribute descriptions!", bindingData.mBinding));
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

		// 2. Compile the array of vertex input attribute descriptions
		//  They will reference the bindings created in step 1.
		for (auto& attribData : _Config.mInputBindingLocations) {
			result.mVertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription()
				.setBinding(attribData.mGeneralData.mBinding)
				.setLocation(attribData.mMemberMetaData.mLocation)
				.setFormat(attribData.mMemberMetaData.mFormat.mFormat)
				.setOffset(static_cast<uint32_t>(attribData.mMemberMetaData.mOffset))
			);
		}

		// 3. With the data from 1. and 2., create the complete vertex input info struct, passed to the pipeline creation
		result.mPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(static_cast<uint32_t>(result.mVertexInputBindingDescriptions.size()))
			.setPVertexBindingDescriptions(result.mVertexInputBindingDescriptions.data())
			.setVertexAttributeDescriptionCount(static_cast<uint32_t>(result.mVertexInputAttributeDescriptions.size()))
			.setPVertexAttributeDescriptions(result.mVertexInputAttributeDescriptions.data());

		// 4. Set how the data (from steps 1.-3.) is to be interpreted (e.g. triangles, points, lists, patches, etc.)
		result.mInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(to_vk_primitive_topology(_Config.mPrimitiveTopology))
			.setPrimitiveRestartEnable(VK_FALSE);

		// 5. Compile and store the shaders:
		for (auto& shaderInfo : _Config.mShaderInfos) {
			// 5.1 Compile the shader
			result.mShaders.push_back(std::move(shader::create(shaderInfo)));
			assert(result.mShaders.back().has_been_built());
			// 5.2 Combine
			result.mShaderStageCreateInfos.push_back(vk::PipelineShaderStageCreateInfo{}
				.setStage(to_vk_shader_stage(result.mShaders.back().info().mShaderType))
				.setModule(result.mShaders.back().handle())
				.setPName(result.mShaders.back().info().mEntryPoint.c_str())
			);
		}

		// 6. Viewport configuration
		{
			// 6.1 Viewport and depth configuration(s):
			for (auto& vp : _Config.mViewportDepthConfig) {
				result.mViewports.push_back(vk::Viewport{}
					.setX(vp.x())
					.setY(vp.y())
					.setWidth(vp.width())
					.setHeight(vp.height())
					.setMinDepth(vp.min_depth())
					.setMaxDepth(vp.max_depth())
				);
			}
			// 6.2 Skip scissors for now
			// TODO: Add scissors later
			// 6.3 Add everything together
			result.mViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{}
				.setViewportCount(static_cast<uint32_t>(result.mViewports.size()))
				.setPViewports(result.mViewports.data())
				.setScissorCount(static_cast<uint32_t>(result.mScissors.size()))
				.setPScissors(result.mScissors.data());
		}

		// 7. Rasterization state
		result.mRasterizationStateCreateInfo =  vk::PipelineRasterizationStateCreateInfo{}
			// Various, but important settings:
			.setRasterizerDiscardEnable(to_vk_bool(_Config.mRasterizerGeometryMode == rasterizer_geometry_mode::discard_geometry))
			.setPolygonMode(to_vk_polygon_mode(_Config.mPolygonDrawingModeAndConfig.drawing_mode()))
			.setLineWidth(_Config.mPolygonDrawingModeAndConfig.line_width())
			.setCullMode(to_vk_cull_mode(_Config.mCullingMode))
			.setFrontFace(to_vk_front_face(_Config.mFrontFaceWindingOrder.winding_order_of_front_faces()))
			// Depth-related settings:
			.setDepthClampEnable(to_vk_bool(_Config.mDepthSettings.is_clamp_to_frustum_enabled()))
			.setDepthBiasEnable(to_vk_bool(_Config.mDepthSettings.is_depth_bias_enabled()))
			.setDepthBiasConstantFactor(_Config.mDepthSettings.bias_constant_factor())
			.setDepthBiasClamp(_Config.mDepthSettings.bias_clamp_value())
			.setDepthBiasSlopeFactor(_Config.mDepthSettings.bias_slope_factor());

		// TODO: Proceed here

		// Maybe alter the config?!
		if (_AlterConfigBeforeCreation.mFunction) {
			_AlterConfigBeforeCreation.mFunction(result);
		}

		// PIPELINE CREATION, a.k.a. putting it all together:
		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			// 1., 2., and 3.
			.setPVertexInputState(&result.mPipelineVertexInputStateCreateInfo)
			// 4.
			.setPInputAssemblyState(&result.mInputAssemblyStateCreateInfo)
			// 5.
			.setStageCount(static_cast<uint32_t>(result.mShaderStageCreateInfos.size()))
			.setPStages(result.mShaderStageCreateInfos.data())
			// 6.
			.setPViewportState(&result.mViewportStateCreateInfo)
			// 7.
			.setPRasterizationState(&result.mRasterizationStateCreateInfo)
			// TODO: Proceed here
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

}
