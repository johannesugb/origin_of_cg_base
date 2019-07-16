#pragma once

namespace cgb
{
	//extern descriptor_set_layout layout_for(std::initializer_list<binding_data> pBindings);






	// End of recursive variadic template handling
	void add_config(graphics_pipeline_config& pConfig) { /* We're done here. */ }

	// Add a specific pipeline setting to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, pipeline_settings pSetting, Ts... args)
	{
		pConfig.mPipelineSettings |= pSetting;
		add_config(config, args...);
	}

	// Add an input binding location to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, input_binding_location_data pInputBinding, Ts... args)
	{
		pConfig.mInputBindingLocations.push_back(std::move(pInputBinding));
		add_config(config, args...);
	}

	// Add a shader to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, shader_info pShaderInfo, Ts... args)
	{
		pConfig.mShaderInfos.push_back(std::move(pShaderInfo));
		add_config(config, args...);
	}

	// Add a resource binding to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, binding_data pResourceBinding, Ts... args)
	{
		pConfig.mResourceBindings.push_back(std::move(pResourceBinding));
		add_config(config, args...);
	}

	// Set the depth test behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, depth_test pDepthTestConfig, Ts... args)
	{
		pConfig.mDepthTestConfig = std::move(pDepthTestConfig);
		add_config(config, args...);
	}

	// Set the depth write behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, depth_write pDepthWriteConfig, Ts... args)
	{
		pConfig.mDepthWriteConfig = std::move(pDepthWriteConfig);
		add_config(config, args...);
	}

	// Add a viewport, depth, and scissors entry to the pipeline configuration
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, viewport_depth_scissors_config pViewportDepthScissorsConfig, Ts... args)
	{
		pConfig.mViewportDepthConfig.push_back(std::move(pViewportDepthScissorsConfig));
		add_config(config, args...);
	}

	// Set the culling mode in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, culling_mode pCullingMode, Ts... args)
	{
		pConfig.mCullingMode = pCullingMode;
		add_config(config, args...);
	}

	// Set the definition of front faces in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, front_face pFrontFace, Ts... args)
	{
		pConfig.mFrontFaceWindingOrder = std::move(pFrontFace);
		add_config(config, args...);
	}

	// Set how to draw polygons in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, polygon_drawing pPolygonDrawingConfig, Ts... args)
	{
		pConfig.mPolygonDrawingModeAndConfig = std::move(pPolygonDrawingConfig);
		add_config(config, args...);
	}

	// Set how the rasterizer handles geometry in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, rasterizer_geometry_mode pRasterizerGeometryMode, Ts... args)
	{
		pConfig.mRasterizerGeometryMode = pRasterizerGeometryMode;
		add_config(config, args...);
	}

	// Sets if there should be some special depth handling in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& pConfig, depth_settings pDepthSettings, Ts... args)
	{
		pConfig.mDepthSettings = pDepthSettings;
		add_config(config, args...);
	}

	// Conveniently construct a pipeline with the given settings
	template <typename... Ts>
	graphics_pipeline graphics_pipeline_for(Ts... args)
	{
		graphics_pipeline_config config;
		add_config(config, args...);

		return graphics_pipeline::create(config);
	}
}