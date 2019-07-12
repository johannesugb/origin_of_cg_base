#pragma once

namespace cgb
{
	enum struct pipeline_settings
	{
		force_new_pipe			= 0x0001,
		fail_if_not_reusable	= 0x0002,
	};

	struct shader_info
	{
		std::string mPath;
		cgb::shader_type mShaderType;
		std::string mEntryPoint;
		bool mDontMonitorFile;
	};

	static bool operator ==(const shader_info& first, const shader_info& second)
	{
		return first.mPath == second.mPath && first.mEntryPoint == second.mEntryPoint;
	}

	shader_info shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false, std::optional<cgb::shader_type> pShaderType = {})
	{
		pPath = trim_spaces(pPath);
		if (!pShaderType.has_value()) {
			// "classical" shaders
			     if (pPath.ends_with(".vert"))	{ pShaderType = cgb::shader_type::vertex; }
			else if (pPath.ends_with(".tesc"))	{ pShaderType = cgb::shader_type::tessellation_control; }
			else if (pPath.ends_with(".tese"))	{ pShaderType = cgb::shader_type::tessellation_evaluation; }
			else if (pPath.ends_with(".geom"))	{ pShaderType = cgb::shader_type::geometry; }
			else if (pPath.ends_with(".frag"))	{ pShaderType = cgb::shader_type::fragment; }
			else if (pPath.ends_with(".comp"))	{ pShaderType = cgb::shader_type::compute; }
			// ray tracing shaders
			else if (pPath.ends_with(".rgen"))	{ pShaderType = cgb::shader_type::ray_generation; }
			else if (pPath.ends_with(".rahit"))	{ pShaderType = cgb::shader_type::any_hit; }
			else if (pPath.ends_with(".rchit"))	{ pShaderType = cgb::shader_type::closest_hit; }
			else if (pPath.ends_with(".rmiss"))	{ pShaderType = cgb::shader_type::miss; }
			else if (pPath.ends_with(".rint"))	{ pShaderType = cgb::shader_type::intersection; }
			// callable shader
			else if (pPath.ends_with(".call"))	{ pShaderType = cgb::shader_type::callable; }
			// mesh shaders
			else if (pPath.ends_with(".task"))	{ pShaderType = cgb::shader_type::task; }
			else if (pPath.ends_with(".mesh"))	{ pShaderType = cgb::shader_type::mesh; }
		}

		if (!pShaderType.has_value()) {
			throw std::runtime_error("No shader type set and could not infer it from the file ending.");
		}

		return shader_info
		{
			std::move(pPath),
			pShaderType.value(),
			std::move(pEntryPoint),
			pDontMonitorFile
		};
	}
	

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

			auto pipeLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
				.setPSetLayouts()

			//cgb::context().logical_device().createPipelineLayoutUnique()

			auto createInfo = vk::ComputePipelineCreateInfo()
				.setFlags(mPipelineCreateFlags)
				.setStage(stageInfo)
				.setLayout()
			cgb::context().logical_device().createComputePipelinesUnique(nullptr, {});
		}

	private:
		vk::PipelineCreateFlagBits mPipelineCreateFlags;
		pipeline_settings mPipelineSettings;
		std::vector<shader_info> mShaderInfos;
		std::vector<shader> mShaders;
		context_specific_function<void(graphics_pipeline&)>
	};

	


	auto pipe = compute_pipe_for(
		compute_shader("asdf.comp.spv"),
		pipeline_settings::force_new_pipe,
		pipeline_settings::fail_if_not_reusable,

		);
}