#pragma once

namespace cgb
{
	//extern descriptor_set_layout layout_for(std::initializer_list<binding_data> pBindings);







	void add_config(graphics_pipeline_config& pConfig) { /* We're done here. */ }

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, pipeline_settings pSetting, Ts... args)
	{
		pConfig.mPipelineSettings |= pSetting;
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, input_binding_location_data pInputBinding, Ts... args)
	{
		pConfig.mInputBindingLocations.push_back(std::move(pInputBinding));
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, shader_info pShaderInfo, Ts... args)
	{
		pConfig.mShaderInfos.push_back(std::move(pShaderInfo));
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, binding_data pResourceBinding, Ts... args)
	{
		pConfig.mResourceBindings.push_back(std::move(pResourceBinding));
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, depth_test pDepthTestConfig, Ts... args)
	{
		pConfig.mDepthTestConfig = std::move(pDepthTestConfig);
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, depth_write pDepthWriteConfig, Ts... args)
	{
		pConfig.mDepthWriteConfig = std::move(pDepthWriteConfig);
		add_config(config, args...);
	}

	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, viewport_depth_scissors_config pViewportDepthScissorsConfig, Ts... args)
	{
		pConfig.mViewportDepthConfig.push_back(std::move(pViewportDepthScissorsConfig));
		add_config(config, args...);
	}

	template <typename... Ts>
	graphics_pipeline graphics_pipeline_for(Ts... args)
	{
		graphics_pipeline_config config;
		add_config(config, args...);

		return graphics_pipeline::create(config);
	}
}