#include "vulkan_pipeline.h"

#include <fstream>

#include "vulkan_render_object.h"

namespace cgb {

	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	vulkan_pipeline::vulkan_pipeline(vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, std::vector<std::shared_ptr<vulkan_resource_bundle_layout>> resourceBundleLayout) :
		mRenderPass(renderPass), mViewport(viewport), mScissor(scissor), mMsaaSamples(msaaSamples), mResourceBundleLayouts(resourceBundleLayout)
	{
		auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional
		mColorBlendAttachments.push_back(colorBlendAttachment);

		mColorBlending = {};
		mColorBlending.logicOpEnable = VK_FALSE;
		mColorBlending.logicOp = vk::LogicOp::eCopy; // Optional
		mColorBlending.blendConstants[0] = 0.0f; // Optional
		mColorBlending.blendConstants[1] = 0.0f; // Optional
		mColorBlending.blendConstants[2] = 0.0f; // Optional
		mColorBlending.blendConstants[3] = 0.0f; // Optional
	}

	vulkan_pipeline::vulkan_pipeline(std::vector<std::shared_ptr<vulkan_resource_bundle_layout>> resourceBundleLayout, size_t pushConstantsSize) :
		mResourceBundleLayouts(resourceBundleLayout), mPushConstantsSize(pushConstantsSize)
	{
	}

	vulkan_pipeline::~vulkan_pipeline()
	{
		cleanup();
	}

	void vulkan_pipeline::cleanup()
	{
		if (initialized) {
			vulkan_context::instance().device.destroyPipeline(mPipeline, nullptr);
			vulkan_context::instance().device.destroyPipelineLayout(mPipelineLayout, nullptr);
		}
	}

	void cgb::vulkan_pipeline::bake()
	{
		// create compute pipeline if a compute module is contained in the shader modules
		shader_module compMod("", ShaderStageFlagBits::eCompute); // empty compute module for comparison
		if (std::find(mShaderModules.begin(), mShaderModules.end(), compMod) != mShaderModules.end() && mShaderModules.size() == 1) {
			recreate_compute();
		}
		else {
			create_graphics_pipeline();
		}
		initialized = true;
	}

	void vulkan_pipeline::recreate_compute()
	{
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		vk::ComputePipelineCreateInfo pipelineInfo = {};
		fill_compute_structures(pipelineLayoutInfo, pipelineInfo);
		recreate_compute(pipelineLayoutInfo, pipelineInfo);
	}

	void vulkan_pipeline::recreate_compute(vk::PipelineLayoutCreateInfo pipelineLayoutInfo, vk::ComputePipelineCreateInfo pipelineInfo)
	{
		cleanup();
		if (vulkan_context::instance().device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &mPipelineLayout) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		pipelineInfo.layout = mPipelineLayout;

		if (vulkan_context::instance().device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr, &mPipeline) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// this has to be always present for a compute pipeline, otherwise it would fail before
		vulkan_context::instance().device.destroyShaderModule(pipelineInfo.stage.module, nullptr);

		initialized = true;
	}

	void vulkan_pipeline::fill_compute_structures(vk::PipelineLayoutCreateInfo& pipelineLayoutInfo, vk::ComputePipelineCreateInfo& pipelineInfo)
	{
		assert(mShaderModules.size() == 1);
		// shaders
		auto compShaderStageInfo = build_shader_stage_create_info(mShaderModules[0]);

		// TODO replace with ressource
		mPushConstantRanges.clear();
		if (mPushConstantsSize > 0) {
			vk::PushConstantRange pushConstantRange = {};
			pushConstantRange.offset = 0;
			pushConstantRange.size = mPushConstantsSize;
			pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
			mPushConstantRanges.push_back(pushConstantRange);
		}

		mTempLayouts.resize(mResourceBundleLayouts.size());
		std::transform(mResourceBundleLayouts.begin(), mResourceBundleLayouts.end(), mTempLayouts.begin(), [](auto rBLayout) { return rBLayout->get_descriptor_set_layout(); });
		pipelineLayoutInfo = {};
		pipelineLayoutInfo.setLayoutCount = mTempLayouts.size(); // Optional
		pipelineLayoutInfo.pSetLayouts = mTempLayouts.data(); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = mPushConstantRanges.size(); // Optional
		pipelineLayoutInfo.pPushConstantRanges = mPushConstantRanges.data(); // Optional

		// finally create compute pipeline
		pipelineInfo = {};
		pipelineInfo.stage = compShaderStageInfo;
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
	}


	void vulkan_pipeline::create_graphics_pipeline() {
		// shaders
		//auto vertShaderCode = readFile("Shader/vert.spv");
		//auto fragShaderCode = readFile("Shader/frag.spv");
		cleanup();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(mShaderModules.size());
		for (int i = 0; i < mShaderModules.size(); i++) {
			shaderStages[i] = build_shader_stage_create_info(mShaderModules[i]);
		}

		// fixed/configureable functions
		auto bindings = std::vector<vk::VertexInputBindingDescription>(mAttrDescBindings.size());
		auto tfFuncAttBind = [](std::shared_ptr<vulkan_attribute_description_binding> attrBind) { return attrBind->get_binding_description(); };
		std::transform(mAttrDescBindings.begin(), mAttrDescBindings.end(), bindings.begin(), tfFuncAttBind);

		auto attributeDescriptions = std::vector<vk::VertexInputAttributeDescription>();
		for (auto attrDescBind : mAttrDescBindings) {
			auto attrDescList = attrDescBind->get_attribute_descriptions();
			attributeDescriptions.insert(attributeDescriptions.end(), attrDescList.begin(), attrDescList.end());
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.vertexBindingDescriptionCount = bindings.size();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindings.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		uint32_t viewportCount = 1;
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.viewportCount = viewportCount;
		viewportState.pViewports = &mViewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &mScissor;

		vk::PipelineViewportShadingRateImageStateCreateInfoNV shadingRateImage = {};
		shadingRateImage.shadingRateImageEnable = VK_TRUE;
		shadingRateImage.viewportCount = viewportCount;

		std::vector<vk::ShadingRatePaletteEntryNV>  shadingRatePaletteEntriesAll;
		//shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X4Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X2Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer1X2Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X1Pixels);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPerPixel);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e2InvocationsPerPixel);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e4InvocationsPerPixel);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e8InvocationsPerPixel);
		shadingRatePaletteEntriesAll.push_back(vk::ShadingRatePaletteEntryNV::e16InvocationsPerPixel);

		std::vector<vk::ShadingRatePaletteEntryNV>  shadingRatePaletteEntries;
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		//shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
		shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPerPixel);


		vk::ShadingRatePaletteNV shadingRatePalette = {};
		shadingRatePalette.shadingRatePaletteEntryCount = shadingRatePaletteEntriesAll.size();
		shadingRatePalette.pShadingRatePaletteEntries = shadingRatePaletteEntriesAll.data();

		shadingRateImage.pShadingRatePalettes = &shadingRatePalette;

		if (vulkan_context::instance().shadingRateImageSupported) {
			viewportState.pNext = &shadingRateImage;
		}

		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = mMsaaSamples;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		vk::DynamicState dynamicStates[] = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eLineWidth
		};

		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		// TODO replace with ressource
		vk::PushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushUniforms);
		pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

		std::vector<vk::DescriptorSetLayout> layouts(mResourceBundleLayouts.size());
		std::transform(mResourceBundleLayouts.begin(), mResourceBundleLayouts.end(), layouts.begin(), [](auto rBLayout) { return rBLayout->get_descriptor_set_layout(); });

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.setLayoutCount = layouts.size(); // Optional
		pipelineLayoutInfo.pSetLayouts = layouts.data(); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

		if (vulkan_context::instance().device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &mPipelineLayout) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = vk::CompareOp::eLess;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

								// finally create graphics pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		mColorBlending.attachmentCount = mColorBlendAttachments.size();
		mColorBlending.pAttachments = mColorBlendAttachments.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &mColorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		pipelineInfo.layout = mPipelineLayout;

		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		pipelineInfo.pDepthStencilState = &depthStencil;

		if (vulkan_context::instance().device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &mPipeline) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		for (auto shaderStage : shaderStages) {
			vulkan_context::instance().device.destroyShaderModule(shaderStage.module, nullptr);
		}
	}

	vk::ShaderModule vulkan_pipeline::create_shader_module(const std::vector<char>& code) {
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		vk::ShaderModule shaderModule;
		if (vulkan_context::instance().device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}


	void cgb::vulkan_pipeline::add_shader(ShaderStageFlagBits shaderStage, const std::string & shaderFilename)
	{
		shader_module mod(shaderFilename, shaderStage);
		check_shader_stage_present(mod);
		mShaderModules.push_back(mod);
	}

	void cgb::vulkan_pipeline::add_shader(ShaderStageFlagBits shaderStage, std::shared_ptr<vk::PipelineShaderStageCreateInfo> shaderModule)
	{
		shader_module mod(shaderModule, shaderStage);
		check_shader_stage_present(mod);
		mShaderModules.push_back(mod);
	}

	void cgb::vulkan_pipeline::check_shader_stage_present(shader_module mod)
	{
		// each module is only allowed once
		if (std::find(mShaderModules.begin(), mShaderModules.end(), mod) != mShaderModules.end()) {
			std::string stage = "";
			switch (mod.shaderStage)
			{
			case ShaderStageFlagBits::eVertex: stage = "VK_SHADER_STAGE_VERTEX_BIT"; break;
			case ShaderStageFlagBits::eTessellationControl: stage = "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT"; break;
			case ShaderStageFlagBits::eTessellationEvaluation: stage = "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT"; break;
			case ShaderStageFlagBits::eGeometry: stage = "VK_SHADER_STAGE_GEOMETRY_BIT"; break;
			case ShaderStageFlagBits::eFragment: stage = "VK_SHADER_STAGE_FRAGMENT_BIT"; break;
			case ShaderStageFlagBits::eCompute: stage = "VK_SHADER_STAGE_COMPUTE_BIT"; break;
			case ShaderStageFlagBits::eAllGraphics: stage = "VK_SHADER_STAGE_ALL_GRAPHICS"; break;
			case ShaderStageFlagBits::eAll: stage = "VK_SHADER_STAGE_ALL"; break;
			case ShaderStageFlagBits::eRaygenNV: stage = "VK_SHADER_STAGE_RAYGEN_BIT_NV"; break;
			case ShaderStageFlagBits::eAnyHitNV: stage = "VK_SHADER_STAGE_ANY_HIT_BIT_NV"; break;
			case ShaderStageFlagBits::eClosestHitNV: stage = "VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV"; break;
			case ShaderStageFlagBits::eMissNV: stage = "VK_SHADER_STAGE_MISS_BIT_NV"; break;
			case ShaderStageFlagBits::eIntersectionNV: stage = "VK_SHADER_STAGE_INTERSECTION_BIT_NV"; break;
			case ShaderStageFlagBits::eCallableNV: stage = "VK_SHADER_STAGE_CALLABLE_BIT_NV"; break;
			case ShaderStageFlagBits::eTaskNV: stage = "VK_SHADER_STAGE_TASK_BIT_NV"; break;
			case ShaderStageFlagBits::eMeshNV: stage = "VK_SHADER_STAGE_MESH_BIT_NV"; break;
			};

			throw std::runtime_error("failed to add shader module! Module with stage " + stage + " has already been added to this pipeline");
		}

		shader_module compMod("", ShaderStageFlagBits::eCompute); // empty compute module for comparison

		// if pipeline already contains a compute module, no other modules allowed
		if (std::find(mShaderModules.begin(), mShaderModules.end(), compMod) != mShaderModules.end()) {
			throw std::runtime_error("failed to add shader module! The pipeline has already a compute module and is not allowed to have a compute module and another module.");
		}

		// if module to add is a compute module, no other modules allowed
		if (mod.shaderStage == ShaderStageFlagBits::eCompute && !mShaderModules.empty()) {
			throw std::runtime_error("failed to add shader module! The pipeline has already another module, so a compute module cannot be added.");
		}
	}

	vk::PipelineShaderStageCreateInfo cgb::vulkan_pipeline::build_shader_stage_create_info(shader_module & mod)
	{
		vk::PipelineShaderStageCreateInfo shaderStage;
		if (!mod.shaderModule) {
			auto shaderCode = readFile(mod.shaderFilename);
			auto shaderModule = create_shader_module(shaderCode);

			vk::PipelineShaderStageCreateInfo shaderStageInfo = {};
			shaderStageInfo.stage = static_cast<vk::ShaderStageFlagBits>(mod.shaderStage);
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = "main";

			shaderStage = shaderStageInfo;
		}
		else {
			shaderStage = *mod.shaderModule;
		}
		return shaderStage;
	}
}