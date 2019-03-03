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

	vulkan_pipeline::vulkan_pipeline(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout) :
		mRenderPass(renderPass)
	{
		create_graphics_pipeline(vertexShaderFilename, fragmentShaderFilename, viewport, scissor, msaaSamples, descriptorSetLayout);
		initialized = true;
	}

	vulkan_pipeline::vulkan_pipeline(const std::string & filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize) :
		mComputeFilename(filename), mDescriptorSetLayouts(descriptorSetLayouts), mPushConstantsSize(pushConstantsSize)
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
		// shaders
		auto compShaderCode = readFile(mComputeFilename);

		vk::ShaderModule compShaderModule;

		compShaderModule = create_shader_module(compShaderCode);

		vk::PipelineShaderStageCreateInfo compShaderStageInfo = {};
		compShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		compShaderStageInfo.module = compShaderModule;
		compShaderStageInfo.pName = "main";

		// TODO replace with ressource
		mPushConstantRanges.clear();
		vk::PushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = mPushConstantsSize;
		pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
		mPushConstantRanges.push_back(pushConstantRange);

		pipelineLayoutInfo = {};
		pipelineLayoutInfo.setLayoutCount = mDescriptorSetLayouts.size(); // Optional
		pipelineLayoutInfo.pSetLayouts = mDescriptorSetLayouts.data(); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = mPushConstantRanges.size(); // Optional
		pipelineLayoutInfo.pPushConstantRanges = mPushConstantRanges.data(); // Optional

		// finally create compute pipeline
		pipelineInfo = {};
		pipelineInfo.stage = compShaderStageInfo;
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
	}


	void vulkan_pipeline::create_graphics_pipeline(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout) {
		// shaders
		//auto vertShaderCode = readFile("Shader/vert.spv");
		//auto fragShaderCode = readFile("Shader/frag.spv");

		auto vertShaderCode = readFile(vertexShaderFilename);
		auto fragShaderCode = readFile(fragmentShaderFilename);

		vk::ShaderModule vertShaderModule;
		vk::ShaderModule fragShaderModule;

		vertShaderModule = create_shader_module(vertShaderCode);
		fragShaderModule = create_shader_module(fragShaderCode);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// fixed/configureable functions
		// TODO extract into own vertex structure
		auto bindingDescription = Vertex::getBindingDescription();
		auto bindingDescription2 = Vertex::getBindingDescription2();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		std::vector<vk::VertexInputBindingDescription> bindings = { bindingDescription, bindingDescription2 };

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
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

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
		shadingRatePalette.shadingRatePaletteEntryCount = shadingRatePaletteEntries.size();
		shadingRatePalette.pShadingRatePaletteEntries = shadingRatePaletteEntries.data();

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
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd; // Optional

		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

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

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
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
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
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

		vulkan_context::instance().device.destroyShaderModule(fragShaderModule, nullptr);
		vulkan_context::instance().device.destroyShaderModule(vertShaderModule, nullptr);
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

}