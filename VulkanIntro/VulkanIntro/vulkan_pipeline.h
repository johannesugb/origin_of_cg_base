#pragma once

#include "vkContext.h"

class vulkan_pipeline
{
public:
	vulkan_pipeline(vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);
	virtual ~vulkan_pipeline();

	vk::Pipeline  get_graphics_pipeline() { return mGraphicsPipeline; }
	vk::PipelineLayout get_pipeline_layout() { return mPipelineLayout; }

private:
	vk::Pipeline mGraphicsPipeline;
	vk::PipelineLayout mPipelineLayout;

	vk::RenderPass mRenderPass;

	void create_graphics_pipeline(vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);
	vk::ShaderModule create_shader_module(const std::vector<char>& code);
};

