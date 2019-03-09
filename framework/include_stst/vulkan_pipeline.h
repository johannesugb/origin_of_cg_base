#pragma once

#include "vulkan_context.h"

#include "vulkan_attribute_description_binding.h"

namespace cgb {

	enum class ShaderStageFlagBits // : std::underlying_type_t<vk::ShaderStageFlagBits>
	{
		eVertex = VK_SHADER_STAGE_VERTEX_BIT,
		eTessellationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		eTessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		eGeometry = VK_SHADER_STAGE_GEOMETRY_BIT,
		eFragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		eCompute = VK_SHADER_STAGE_COMPUTE_BIT,
		eAllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS,
		eAll = VK_SHADER_STAGE_ALL,
		eRaygenNV = VK_SHADER_STAGE_RAYGEN_BIT_NV,
		eAnyHitNV = VK_SHADER_STAGE_ANY_HIT_BIT_NV,
		eClosestHitNV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
		eMissNV = VK_SHADER_STAGE_MISS_BIT_NV,
		eIntersectionNV = VK_SHADER_STAGE_INTERSECTION_BIT_NV,
		eCallableNV = VK_SHADER_STAGE_CALLABLE_BIT_NV,
		eTaskNV = VK_SHADER_STAGE_TASK_BIT_NV,
		eMeshNV = VK_SHADER_STAGE_MESH_BIT_NV
	};

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
		vulkan_pipeline(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, vk::RenderPass renderPass, vk::Viewport viewport, vk::Rect2D scissor, vk::SampleCountFlagBits msaaSamples, vk::DescriptorSetLayout descriptorSetLayout);

		vulkan_pipeline(const std::string& filename, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts, size_t pushConstantsSize);
		virtual ~vulkan_pipeline();

		vk::Pipeline  get_pipeline() { return mPipeline; }
		vk::PipelineLayout get_pipeline_layout() { return mPipelineLayout; }

		std::vector<vk::DescriptorSetLayout> get_descriptor_set_layouts() { return mDescriptorSetLayouts; }
		void set_descriptor_set_layouts(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts) { mDescriptorSetLayouts = descriptorSetLayouts; }

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

	private:
		vk::Pipeline mPipeline;
		vk::PipelineLayout mPipelineLayout;

		void create_graphics_pipeline();

		vk::ShaderModule create_shader_module(const std::vector<char>& code);

		// pipeline config
		std::string mComputeFilename;
		std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts;
		std::vector<vk::PushConstantRange> mPushConstantRanges;
		size_t mPushConstantsSize;

		// TODO wrap
		std::string mVertexFilename;
		std::string mFragmentFilename;
		vk::RenderPass mRenderPass;
		vk::DescriptorSetLayout mDescriptorSetLayout;

		vk::Viewport mViewport;
		vk::Rect2D mScissor; 
		vk::SampleCountFlagBits mMsaaSamples; 

		std::vector<std::shared_ptr<vulkan_attribute_description_binding>> mAttrDescBindings;
		std::vector<shader_module> mShaderModules;

		bool initialized = false;
		void cleanup();

		void check_shader_stage_present(shader_module mod);
	};
}

