#pragma once

#include "vulkan_context.h"

#include "vulkan_attribute_description_binding.h"
#include "vulkan_resource_bundle_layout.h"

namespace cgb {

	struct shader_module {

		shader_module(const std::string &shaderFilename, ShaderStageFlagBits shaderStage) : shaderFilename(shaderFilename), shaderStage(shaderStage)
		{}

		shader_module(std::shared_ptr<vk::PipelineShaderStageCreateInfo> shaderModule, ShaderStageFlagBits shaderStage) : shaderModule(shaderModule), shaderStage(shaderStage)
		{}

		const std::string shaderFilename;
		ShaderStageFlagBits shaderStage;

		// if this variable is set, it will be used as shaderModule
		// this allows the user to all data as needed for shaders (e.g. extensions with pNext)
		std::shared_ptr<vk::PipelineShaderStageCreateInfo> shaderModule = nullptr;

		// only one module for one stage allowed
		bool operator==(const shader_module& other) const {
			return shaderStage == other.shaderStage;
		}
	};

	class vulkan_pipeline
	{
	public:
		vulkan_pipeline(vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> resourceBundleLayouts);

		vulkan_pipeline(std::vector<std::shared_ptr<vulkan_resource_bundle_layout>> resourceBundleLayouts, size_t pushConstantsSize);
		virtual ~vulkan_pipeline();

		vk::Pipeline  get_pipeline() { return mPipeline; }
		vk::PipelineLayout get_pipeline_layout() { return mPipelineLayout; }

		void bake();
		void recreate_compute();
		void recreate_compute(vk::PipelineLayoutCreateInfo pipelineLayoutInfo, vk::ComputePipelineCreateInfo pipelineInfo);
		void fill_compute_structures(vk::PipelineLayoutCreateInfo& pipelineLayoutInfo, vk::ComputePipelineCreateInfo& pipelineInfo);

		void set_viewport(vk::Viewport viewport) { mViewport = viewport; };
		void set_scissor(vk::Rect2D scissor) { mScissor = scissor; };
		void set_msaa_samples(vk::SampleCountFlagBits msaaSamples) { mMsaaSamples = msaaSamples; };

		void add_attr_desc_binding(std::shared_ptr<vulkan_attribute_description_binding> attrBind) { mAttrDescBindings.push_back(attrBind); }

		void add_shader(ShaderStageFlagBits shaderStage, const std::string &shaderFilename);

		void add_shader(ShaderStageFlagBits shaderStage, std::shared_ptr<vk::PipelineShaderStageCreateInfo> shaderModule);

		void add_resource_bundle_layout(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout) { mResourceBundleLayouts.push_back(resourceBundleLayout); };

		vk::PipelineColorBlendStateCreateInfo& get_color_blend_state() { return mColorBlending; }
		vk::PipelineColorBlendAttachmentState& get_color_blend_attachment_state() { return mColorBlendAttachment; }

	private:
		vk::Pipeline mPipeline;
		vk::PipelineLayout mPipelineLayout;

		void create_graphics_pipeline();

		vk::ShaderModule create_shader_module(const std::vector<char>& code);
		vk::PipelineShaderStageCreateInfo build_shader_stage_create_info(shader_module& mod);

		// pipeline config
		std::vector<vk::PushConstantRange> mPushConstantRanges;
		size_t mPushConstantsSize;

		vk::RenderPass mRenderPass;

		vk::Viewport mViewport;
		vk::Rect2D mScissor; 
		vk::SampleCountFlagBits mMsaaSamples; 
		vk::PipelineColorBlendStateCreateInfo mColorBlending;
		vk::PipelineColorBlendAttachmentState mColorBlendAttachment;

		std::vector<std::shared_ptr<vulkan_attribute_description_binding>> mAttrDescBindings;
		std::vector<shader_module> mShaderModules;
		std::vector<std::shared_ptr<vulkan_resource_bundle_layout>> mResourceBundleLayouts;

		bool initialized = false;
		void cleanup();

		void check_shader_stage_present(shader_module mod);

		// private temporary variables, for filling structures
		std::vector<vk::DescriptorSetLayout> mTempLayouts;
	};
}

