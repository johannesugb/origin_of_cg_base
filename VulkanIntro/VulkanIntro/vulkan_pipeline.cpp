#include "vulkan_pipeline.h"

#include <fstream>

#include "vkRenderObject.h"

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

vulkan_pipeline::vulkan_pipeline(vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout) :
	mRenderPass(renderPass)
{
	create_graphics_pipeline(viewport, scissor, msaaSamples, descriptorSetLayout);
}

vulkan_pipeline::vulkan_pipeline(const std::string & filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize)
{
	create_compute_pipeline(filename, descriptorSetLayouts, pushConstantsSize);
}

vulkan_pipeline::~vulkan_pipeline()
{
	vkContext::instance().device.destroyPipeline(mPipeline, nullptr);
	vkContext::instance().device.destroyPipelineLayout(mPipelineLayout, nullptr);
}

void vulkan_pipeline::create_graphics_pipeline(vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout) {
	// shaders
	auto vertShaderCode = readFile("Shader/vert.spv");
	auto fragShaderCode = readFile("Shader/frag.spv");

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

	vk::VertexInputBindingDescription bindings[] = { bindingDescription, bindingDescription2 };

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = 2;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindings;
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

	std::vector<vk::ShadingRatePaletteEntryNV>  shadingRatePaletteEntries;
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::eNoInvocations);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X4Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer4X2Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer1X2Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPer2X1Pixels);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e1InvocationPerPixel);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e2InvocationsPerPixel);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e4InvocationsPerPixel);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e8InvocationsPerPixel);
	shadingRatePaletteEntries.push_back(vk::ShadingRatePaletteEntryNV::e16InvocationsPerPixel);
	vk::ShadingRatePaletteNV shadingRatePalette = {};
	shadingRatePalette.shadingRatePaletteEntryCount = shadingRatePaletteEntries.size();
	shadingRatePalette.pShadingRatePaletteEntries = shadingRatePaletteEntries.data();

	shadingRateImage.pShadingRatePalettes = &shadingRatePalette;

	viewportState.pNext = &shadingRateImage;

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

	if (vkContext::instance().device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &mPipelineLayout) != vk::Result::eSuccess) {
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

	if (vkContext::instance().device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &mPipeline) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkContext::instance().device.destroyShaderModule(fragShaderModule, nullptr);
	vkContext::instance().device.destroyShaderModule(vertShaderModule, nullptr);
}

void vulkan_pipeline::create_compute_pipeline(const std::string& filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize)
{
	// shaders
	auto compShaderCode = readFile(filename);

	vk::ShaderModule compShaderModule;

	compShaderModule = create_shader_module(compShaderCode);

	vk::PipelineShaderStageCreateInfo compShaderStageInfo = {};
	compShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
	compShaderStageInfo.module = compShaderModule;
	compShaderStageInfo.pName = "main";

	// TODO replace with ressource
	vk::PushConstantRange pushConstantRange = {};
	pushConstantRange.offset = 0;
	pushConstantRange.size = pushConstantsSize;
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

	if (vkContext::instance().device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &mPipelineLayout) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// finally create compute pipeline
	vk::ComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.stage = compShaderStageInfo;
	pipelineInfo.layout = mPipelineLayout;

	pipelineInfo.basePipelineHandle = nullptr; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkContext::instance().device.createComputePipelines(nullptr, 1, &pipelineInfo, nullptr, &mPipeline) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkContext::instance().device.destroyShaderModule(compShaderModule, nullptr);
}

vk::ShaderModule vulkan_pipeline::create_shader_module(const std::vector<char>& code) {
	vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	vk::ShaderModule shaderModule;
	if (vkContext::instance().device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}