namespace cgb
{
	// Set sensible defaults:
	graphics_pipeline_config::graphics_pipeline_config()
		: mPipelineSettings{ pipeline_settings::nothing }
		, mPrimitiveTopology{ primitive_topology::triangles }
		// TODO: Proceed here with defining default parameters
		, mDepthTestConfig(depth_test::enabled())
		, mDepthWriteConfig(depth_write::enabled())
		, mCullingMode(culling_mode::cull_back_faces)
		, mFrontFaceWindingOrder(front_face::define_front_faces_to_be_counter_clockwise())
		, mPolygonDrawingModeAndConfig(polygon_drawing::config_for_filling())
		, mRasterizerGeometryMode(rasterizer_geometry_mode::rasterize_geometry)
		, mDepthSettings(depth_settings::config_nothing_special())
		, mColorBlendingSettings(color_blending_settings::disable_logic_operation())
		
	{
	}
}
