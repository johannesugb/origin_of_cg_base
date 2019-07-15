#pragma once

namespace cgb
{
	enum struct pipeline_settings
	{
		force_new_pipe			= 0x0001,
		fail_if_not_reusable	= 0x0002,
	};


	static bool operator ==(const shader_info& first, const shader_info& second)
	{
		return first.mPath == second.mPath && first.mEntryPoint == second.mEntryPoint;
	}
	
	shader_info vertex_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::vertex); }
	shader_info tessellation_control_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::tessellation_control); }
	shader_info tessellation_evaluation_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::tessellation_evaluation); }
	shader_info geometry_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::geometry); }
	shader_info fragment_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::fragment); }
	shader_info compute_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::compute); }
	shader_info ray_generation_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::ray_generation); }
	shader_info any_hit_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::any_hit); }
	shader_info closest_hit_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::closest_hit); }
	shader_info miss_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::miss); }
	shader_info intersection_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::intersection); }
	shader_info callable_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::callable); }
	shader_info task_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::task); }
	shader_info mesh_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::mesh); }


	class graphics_pipeline
	{
		void create()
		{
			//std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
			//std::transform(std::begin(pShaderInfos), std::end(pShaderInfos),
			//	std::back_inserter(shaderStages),
			//	[](const auto& tpl) {
			//	return vk::PipelineShaderStageCreateInfo()
			//		.setStage(convert(std::get<shader_type>(tpl)))
			//		.setModule(std::get<shader*>(tpl)->handle())
			//		.setPName("main"); // TODO: support different entry points?!
			//});

			//auto shaderStages = vk::PipelineShaderStageCreateInfo()
			//	.set

			assert(mShaderInfos.size() == 1);
			assert(cgb::shader_type::compute == mShaderInfos[0].mShaderType);
			assert(mShaders.size() == 1);
			auto stageInfo = vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(mShaders[0].handle())
				.setPName(mShaderInfos[0].mEntryPoint.c_str());

			//auto pipeLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			//	.setPSetLayouts()

			//cgb::context().logical_device().createPipelineLayoutUnique()

			//auto createInfo = vk::ComputePipelineCreateInfo()
			//	.setFlags(mPipelineCreateFlags)
			//	.setStage(stageInfo)
			//	.setLayout()
			//cgb::context().logical_device().createComputePipelinesUnique(nullptr, {});
		}

	private:
		vk::PipelineCreateFlagBits mPipelineCreateFlags;
		pipeline_settings mPipelineSettings;
		std::vector<shader_info> mShaderInfos;
		std::vector<shader> mShaders;
		//context_specific_function<void(graphics_pipeline&)>
	};

	


	//auto pipe = compute_pipe_for(
	//	compute_shader("asdf.comp.spv"),
	//	pipeline_settings::force_new_pipe,
	//	pipeline_settings::fail_if_not_reusable,

	//	);
}