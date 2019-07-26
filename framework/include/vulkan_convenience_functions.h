#pragma once

namespace cgb
{
	//extern descriptor_set_layout layout_for(std::initializer_list<binding_data> pBindings);




	// End of recursive variadic template handling
	inline void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments) { /* We're done here. */ }

	// Add a specific pipeline setting to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, pipeline_settings _Setting, Ts... args)
	{
		_Config.mPipelineSettings |= _Setting;
		add_config(_Config, _Attachments, args...);
	}

	// Add a complete render pass to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, renderpass _RenderPass, uint32_t _Subpass, Ts... args)
	{
		_Config.mRenderPassSubpass = std::make_tuple(std::move(_RenderPass), _Subpass);
		add_config(_Config, _Attachments, args...);
	}

	// Add a complete render pass to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, renderpass _RenderPass, Ts... args)
	{
		_Config.mRenderPassSubpass = std::make_tuple(std::move(_RenderPass), 0u); // Default to the first subpass if none is specified
		add_config(_Config, _Attachments, args...);
	}

	// Add a renderpass attachment to the (temporary) attachments vector and build renderpass afterwards
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, attachment _Attachment, Ts... args)
	{
		_Attachments->push_back(std::move(_Attachment));
		add_config(_Config, _Attachments, args...);
	}

	// Add an input binding location to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, input_binding_location_data _InputBinding, Ts... args)
	{
		_Config.mInputBindingLocations.push_back(std::move(_InputBinding));
		add_config(_Config, _Attachments, args...);
	}

	// Set the topology of the input attributes
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, primitive_topology _Topology, Ts... args)
	{
		_Config.mPrimitiveTopology = _Topology;
		add_config(_Config, _Attachments, args...);
	}

	// Add a shader to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, shader_info _ShaderInfo, Ts... args)
	{
		_Config.mShaderInfos.push_back(std::move(_ShaderInfo));
		add_config(_Config, _Attachments, args...);
	}

	// Set the depth test behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, depth_test _DepthTestConfig, Ts... args)
	{
		_Config.mDepthTestConfig = std::move(_DepthTestConfig);
		add_config(_Config, _Attachments, args...);
	}

	// Set the depth write behavior in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, depth_write _DepthWriteConfig, Ts... args)
	{
		_Config.mDepthWriteConfig = std::move(_DepthWriteConfig);
		add_config(_Config, _Attachments, args...);
	}

	// Add a viewport, depth, and scissors entry to the pipeline configuration
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, viewport_depth_scissors_config _ViewportDepthScissorsConfig, Ts... args)
	{
		_Config.mViewportDepthConfig.push_back(std::move(_ViewportDepthScissorsConfig));
		add_config(_Config, _Attachments, args...);
	}

	// Set the culling mode in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, culling_mode _CullingMode, Ts... args)
	{
		_Config.mCullingMode = _CullingMode;
		add_config(_Config, _Attachments, args...);
	}

	// Set the definition of front faces in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, front_face _FrontFace, Ts... args)
	{
		_Config.mFrontFaceWindingOrder = std::move(_FrontFace);
		add_config(_Config, _Attachments, args...);
	}

	// Set how to draw polygons in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, polygon_drawing _PolygonDrawingConfig, Ts... args)
	{
		_Config.mPolygonDrawingModeAndConfig = std::move(_PolygonDrawingConfig);
		add_config(_Config, _Attachments, args...);
	}

	// Set how the rasterizer handles geometry in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, rasterizer_geometry_mode _RasterizerGeometryMode, Ts... args)
	{
		_Config.mRasterizerGeometryMode = _RasterizerGeometryMode;
		add_config(_Config, _Attachments, args...);
	}

	// Sets if there should be some special depth handling in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, depth_clamp_bias _DepthSettings, Ts... args)
	{
		_Config.mDepthClampBiasConfig = _DepthSettings;
		add_config(_Config, _Attachments, args...);
	}

	// Sets some color blending parameters in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, color_blending_settings _ColorBlendingSettings, Ts... args)
	{
		_Config.mColorBlendingSettings = _ColorBlendingSettings;
		add_config(_Config, _Attachments, args...);
	}

	// Sets some color blending parameters in the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, color_blending_config _ColorBlendingConfig, Ts... args)
	{
		_Config.mColorBlendingPerAttachment.push_back(std::move(_ColorBlendingConfig));
		add_config(_Config, _Attachments, args...);
	}

	// Add a resource binding to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, binding_data _ResourceBinding, Ts... args)
	{
		_Config.mResourceBindings.push_back(std::move(_ResourceBinding));
		add_config(_Config, _Attachments, args...);
	}

	// Add a resource binding to the pipeline config
	template <typename... Ts>
	void add_config(graphics_pipeline_config& _Config, std::vector<attachment>* _Attachments, push_constant_binding_data _PushConstBinding, Ts... args)
	{
		_Config.mPushConstantsBindings.push_back(std::move(_PushConstBinding));
		add_config(_Config, _Attachments, args...);
	}

	// Conveniently construct a pipeline with the given settings
	template <typename... Ts>
	graphics_pipeline graphics_pipeline_for(Ts... args, cgb::context_specific_function<void(graphics_pipeline&)> _AlterConfigBeforeCreation = {})
	{
		std::vector<attachment> renderPassAttachments;
		// TODO: Proceed here and check if there is data in both, the vector of attachments and the mRenderpass
		//       - If so => throw!
		//       - If not => fine

		graphics_pipeline_config config;
		add_config(config, &renderPassAttachments, args...);

		return graphics_pipeline::create(config, std::move(_AlterConfigBeforeCreation));
	}
}