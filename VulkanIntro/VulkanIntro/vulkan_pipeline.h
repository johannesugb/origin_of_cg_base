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

private:
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;

	vk::RenderPass mRenderPass;

	void create_graphics_pipeline(vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);
	void create_compute_pipeline(const std::string& filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize);

	vk::ShaderModule create_shader_module(const std::vector<char>& code);
};

