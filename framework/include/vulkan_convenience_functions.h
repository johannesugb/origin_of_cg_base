#pragma once

namespace cgb
{
	//extern descriptor_set_layout layout_for(std::initializer_list<binding_data> pBindings);




	// End of recursive variadic template handling
	inline void add_config(graphics_pipeline_config& _Config) { /* We're done here. */ }

	// Add a specific pipeline setting to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, pipeline_settings _Setting, Ts... args)
	{
		_Config.mPipelineSettings |= _Setting;
		add_config(_Config, args...);
	}

	// Add an input binding location to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, input_binding_location_data _InputBinding, Ts... args)
	{
		_Config.mInputBindingLocations.push_back(std::move(_InputBinding));
		add_config(_Config, args...);
	}

	// Set the topology of the input attributes
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, primitive_topology _Topology, Ts... args)
	{
		_Config.mPrimitiveTopology = _Topology;
		add_config(_Config, args...);
	}

	// Add a shader to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, shader_info _ShaderInfo, Ts... args)
	{
		_Config.mShaderInfos.push_back(std::move(_ShaderInfo));
		add_config(_Config, args...);
	}

	// Set the depth test behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, depth_test _DepthTestConfig, Ts... args)
	{
		_Config.mDepthTestConfig = std::move(_DepthTestConfig);
		add_config(_Config, args...);
	}

	// Set the depth write behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, depth_write _DepthWriteConfig, Ts... args)
	{
		_Config.mDepthWriteConfig = std::move(_DepthWriteConfig);
		add_config(_Config, args...);
	}

	// Add a viewport, depth, and scissors entry to the pipeline configuration
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, viewport_depth_scissors_config _ViewportDepthScissorsConfig, Ts... args)
	{
		_Config.mViewportDepthConfig.push_back(std::move(_ViewportDepthScissorsConfig));
		add_config(_Config, args...);
	}

	// Set the culling mode in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, culling_mode _CullingMode, Ts... args)
	{
		_Config.mCullingMode = _CullingMode;
		add_config(_Config, args...);
	}

	// Set the definition of front faces in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, front_face _FrontFace, Ts... args)
	{
		_Config.mFrontFaceWindingOrder = std::move(_FrontFace);
		add_config(_Config, args...);
	}

	// Set how to draw polygons in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, polygon_drawing _PolygonDrawingConfig, Ts... args)
	{
		_Config.mPolygonDrawingModeAndConfig = std::move(_PolygonDrawingConfig);
		add_config(_Config, args...);
	}

	// Set how the rasterizer handles geometry in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, rasterizer_geometry_mode _RasterizerGeometryMode, Ts... args)
	{
		_Config.mRasterizerGeometryMode = _RasterizerGeometryMode;
		add_config(_Config, args...);
	}

	// Sets if there should be some special depth handling in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, depth_settings _DepthSettings, Ts... args)
	{
		_Config.mDepthSettings = _DepthSettings;
		add_config(_Config, args...);
	}

	// Sets some color blending parameters in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, color_blending_settings _ColorBlendingSettings, Ts... args)
	{
		_Config.mColorBlendingSettings = _ColorBlendingSettings;
		add_config(_Config, args...);
	}

	// Sets some color blending parameters in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, color_blending_config _ColorBlendingConfig, Ts... args)
	{
		_Config.mColorBlendingPerAttachment.push_back(std::move(_ColorBlendingConfig));
		add_config(_Config, args...);
	}

	// Add a resource binding to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, binding_data _ResourceBinding, Ts... args)
	{
		_Config.mResourceBindings.push_back(std::move(_ResourceBinding));
		add_config(_Config, args...);
	}

	// Add a resource binding to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, push_constant_binding_data _PushConstBinding, Ts... args)
	{
		_Config.mPushConstantsBindings.push_back(std::move(_PushConstBinding));
		add_config(_Config, args...);
	}

	// Conveniently construct a pipeline with the given settings
	template <typename... Ts>
	graphics_pipeline graphics_pipeline_for(Ts... args, cgb::context_specific_function<void(graphics_pipeline&)> _AlterConfigBeforeCreation = {})
	{
		graphics_pipeline_config config;
		add_config(config, args...);

		return graphics_pipeline::create(config, std::move(_AlterConfigBeforeCreation));
	}
}