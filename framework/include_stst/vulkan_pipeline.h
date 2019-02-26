#pragma once

#include "vkContext.h"

class vulkan_pipeline
{
public:
	vulkan_pipeline(vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);

	vulkan_pipeline(const std::string& filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize);
	virtual ~vulkan_pipeline();

	vk::Pipeline  get_pipeline() { return mPipeline; }
	vk::PipelineLayout get_pipeline_layout() { return mPipelineLayout; }

	std::vector<vk::DescriptorSetLayout> get_descriptor_set_layouts() { return mDescriptorSetLayouts; }
	void set_descriptor_set_layouts(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts) { mDescriptorSetLayouts = descriptorSetLayouts; }

	void recreate_compute();
	void recreate_compute(vk::PipelineLayoutCreateInfo pipelineLayoutInfo, vk::ComputePipelineCreateInfo pipelineInfo);
	void fill_compute_structures(vk::PipelineLayoutCreateInfo& pipelineLayoutInfo, vk::ComputePipelineCreateInfo& pipelineInfo);

private:
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;

	vk::RenderPass mRenderPass;

	void create_graphics_pipeline(vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);

	vk::ShaderModule create_shader_module(const std::vector<char>& code);

	// pipeline config
	std::string mComputeFilename; 
	std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts; 
	std::vector<vk::PushConstantRange> mPushConstantRanges;
	size_t mPushConstantsSize;

	bool initialized = false;
	void cleanup();
};

