#pragma once

namespace cgb
{
	/** Pipeline configuration data: General pipeline settings */
	enum struct pipeline_settings
	{
		nothing					= 0x0000,
		force_new_pipe			= 0x0001,
		fail_if_not_reusable	= 0x0002,
	};

	inline pipeline_settings operator| (pipeline_settings a, pipeline_settings b)
	{
		typedef std::underlying_type<pipeline_settings>::type EnumType;
		return static_cast<pipeline_settings>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
	}

	inline pipeline_settings operator& (pipeline_settings a, pipeline_settings b)
	{
		typedef std::underlying_type<pipeline_settings>::type EnumType;
		return static_cast<pipeline_settings>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
	}

	inline pipeline_settings& operator |= (pipeline_settings& a, pipeline_settings b)
	{
		return a = a | b;
	}

	inline pipeline_settings& operator &= (pipeline_settings& a, pipeline_settings b)
	{
		return a = a & b;
	}

	/** Pipeline configuration data: Depth Test settings */
	struct depth_test
	{
		static depth_test enabled() { return depth_test{ true }; }
		static depth_test disabled() { return depth_test{ false }; }
		bool is_enabled() const { return mEnabled; }
		bool mEnabled;
	};

	/** Pipeline configuration data: Depth Write settings */
	struct depth_write
	{
		static depth_write enabled() { return depth_write{ true }; }
		static depth_write disabled() { return depth_write{ false }; }
		bool is_enabled() const { return mEnabled; }
		bool mEnabled;
	};

	/** Viewport position and extent */
	struct viewport_depth_scissors_config
	{
		static viewport_depth_scissors_config from_pos_extend_and_depth(int32_t x, int32_t y, int32_t width, int32_t height, float minDepth, float maxDepth) 
		{ 
			return viewport_depth_scissors_config{ 
				{static_cast<float>(x), static_cast<float>(y)},
				{static_cast<float>(width), static_cast<float>(height)}, 
				minDepth, maxDepth,
				{x, y},
				{width, height}
			}; 
		}

		static viewport_depth_scissors_config from_pos_and_extent(int32_t x, int32_t y, int32_t width, int32_t height) 
		{ 
			return viewport_depth_scissors_config{ 
				{static_cast<float>(x), static_cast<float>(y)},
				{static_cast<float>(width), static_cast<float>(height)}, 
				0.0f, 1.0f,
				{x, y},
				{width, height}
			}; 
		}

		static viewport_depth_scissors_config from_extent(int32_t width, int32_t height) 
		{ 
			return viewport_depth_scissors_config{ 
				{0.0f, 0.0f},
				{static_cast<float>(width), static_cast<float>(height)}, 
				0.0f, 1.0f,
				{0, 0},
				{width, height}
			}; 
		}

		static viewport_depth_scissors_config from_window(const window* window) 
		{ 
			auto dimensions = window->resolution(); // TODO: Is this the right one?
			return viewport_depth_scissors_config{ 
				{0.0f, 0.0f},
				{static_cast<float>(dimensions.x), static_cast<float>(dimensions.y)}, 
				0.0f, 1.0f,
				{0, 0},
				{static_cast<int32_t>(dimensions.x), static_cast<int32_t>(dimensions.y)}
			}; 
		}

		const auto& position() const { return mPosition; }
		auto x() const { return mPosition.x; }
		auto y() const { return mPosition.y; }
		const auto& dimensions() const { return mDimensions; }
		auto width() const { return mDimensions.x; }
		auto height() const { return mDimensions.y; }
		auto min_depth() const { return mMinDepth; }
		auto max_depth() const { return mMaxDepth; }
		const auto& scissor_offset() const { return mScissorOffset; }
		auto scissor_x() const { return mScissorOffset.x; }
		auto scissor_y() const { return mScissorOffset.y; }
		const auto& scissor_extent() const { return mScissorExtent; }
		auto scissor_width() const { return mScissorExtent.x; }
		auto scissor_height() const { return mScissorExtent.y; }

		glm::vec2 mPosition;
		glm::vec2 mDimensions;
		float mMinDepth;
		float mMaxDepth;
		glm::ivec2 mScissorOffset;
		glm::ivec2 mScissorExtent;
	};

	
	/** Pipeline configuration data: BIG GRAPHICS PIPELINE CONFIG STRUCT */
	struct graphics_pipeline_config
	{
		graphics_pipeline create(
			pipeline_settings pSettings, 
			input_description pInputDescription,
			std::vector<shader> pShaders)
		{
			//std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
			//std::transform(std::begin(pShaderInfos), std::end(pShaderInfos),
			//	std::back_inserter(shaderStages),
			//	[](const auto& tpl) {
			//	return vk::PipelineShaderStageCreateInfo()
			//		.setStage(convert(std::get<pipeline_settings>(tpl)))
			//		.setModule(std::get<shader*>(tpl)->handle())
			//		.setPName("main"); // TODO: support different entry points?!
			//});

			//auto shaderStages = vk::PipelineShaderStageCreateInfo()
			//	.set

			assert(mShaderInfos.size() == 1);
			assert(cgb::pipeline_settings::compute == mShaderInfos[0].mShaderType);
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

		pipeline_settings mPipelineSettings;
		std::vector<input_binding_location_data> mInputBindingLocations;
		std::vector<shader_info> mShaderInfos;
		std::vector<binding_data> mResourceBindings;
		depth_test mDepthTestConfig;
		depth_write mDepthWriteConfig;
		std::vector<viewport_depth_scissors_config> mViewportDepthConfig;
	};



	//auto pipe = compute_pipe_for(
	//	compute_shader("asdf.comp.spv"),
	//	pipeline_settings::force_new_pipe,
	//	pipeline_settings::fail_if_not_reusable,

	//	);
}