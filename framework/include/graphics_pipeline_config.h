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

	/** Pipeline configuration data: Culling Mode */
	enum struct culling_mode
	{
		 disabled,
		 cull_front_faces,
		 cull_back_faces,
		 cull_front_and_back_faces
	};

	/** Winding order of polygons */
	enum struct winding_order
	{
		counter_clockwise,
		clockwise
	};

	/** Pipeline configuration data: Culling Mode */
	struct front_face
	{
		static front_face define_front_faces_to_be_counter_clockwise() { return front_face{ winding_order::counter_clockwise }; }
		static front_face define_front_faces_to_be_clockwise() { return front_face{ winding_order::clockwise }; }

		winding_order winding_order_of_front_faces() const { return mFrontFaces; }
		winding_order winding_order_of_back_faces() const { return mFrontFaces == winding_order::counter_clockwise ? winding_order::clockwise : winding_order::counter_clockwise; }

		winding_order mFrontFaces;
	};

	/** How to draw polygons */
	enum struct polygon_drawing_mode
	{
		fill,
		line,
		point
	};

	/** Pipeline configuration data: Polygon Drawing Mode (and additional settings) */
	struct polygon_drawing
	{
		static polygon_drawing config_for_filling() 
		{ 
			return { polygon_drawing_mode::fill, 1.0f, false, 1.0f }; 
		}
		
		static polygon_drawing config_for_lines(float pLineWidth = 1.0f, bool pDynamic = false) 
		{ 
			return { polygon_drawing_mode::line, pLineWidth, pDynamic, 1.0f }; 
		}
		
		static polygon_drawing config_for_points(float pPointSize = 1.0f) 
		{ 
			return { polygon_drawing_mode::point, 1.0f, false, pPointSize }; 
		}

		auto drawing_mode() const { return mDrawingMode; }
		auto line_width() const { return mLineWidth; }
		auto dynamic_line_width() const { return mDynamicLineWidth; }

		polygon_drawing_mode mDrawingMode;
		float mLineWidth;
		bool mDynamicLineWidth;

		float mPointSize;
	};

	/** How the rasterizer processes geometry */
	enum struct rasterizer_geometry_mode
	{
		rasterize_geometry,
		discard_geometry,
	};

	/** Additional depth-related parameters for the rasterizer */
	struct depth_settings
	{
		static depth_settings config_nothing_special() { return { false, false, 0.0f, 0.0f, 0.0f }; }
		static depth_settings config_enable_depth_bias(float pConstantFactor, float pBiasClamp, float pSlopeFactor) { return { false, true, pConstantFactor, pBiasClamp, pSlopeFactor }; }
		static depth_settings config_enable_clamp_and_depth_bias(float pConstantFactor, float pBiasClamp, float pSlopeFactor) { return { true, true, pConstantFactor, pBiasClamp, pSlopeFactor }; }

		auto is_clamp_to_frustum_enabled() const { return mClampDepthToFrustum; }
		auto is_depth_bias_enabled() const { return mEnableDepthBias; }
		auto bias_constant_factor() const { return mDepthBiasConstantFactor; }
		auto bias_clamp_value() const { return mDepthBiasClamp; }
		auto bias_slope_factor() const { return mDepthBiasSlopeFactor; }

		bool mClampDepthToFrustum;
		bool mEnableDepthBias;
		float mDepthBiasConstantFactor;
		float mDepthBiasClamp;
		float mDepthBiasSlopeFactor;
	};

	/** Reference the separate color channels */
	enum struct color_channel
	{
		none		= 0x0000,
		red			= 0x0001,
		green		= 0x0002,
		blue		= 0x0004,
		alpha		= 0x0008,
		rg			= red | green,
		rgb			= red | green | blue,
		rgba		= red | green | blue | alpha
	};

	inline color_channel operator| (color_channel a, color_channel b)
	{
		typedef std::underlying_type<color_channel>::type EnumType;
		return static_cast<color_channel>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
	}

	inline color_channel operator& (color_channel a, color_channel b)
	{
		typedef std::underlying_type<color_channel>::type EnumType;
		return static_cast<color_channel>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
	}

	inline color_channel& operator |= (color_channel& a, color_channel b)
	{
		return a = a | b;
	}

	inline color_channel& operator &= (color_channel& a, color_channel b)
	{
		return a = a & b;
	}

	enum struct color_blending_mode
	{
		color_blending,
		logic_operation
	};

	/** Different operation types for color blending */
	enum struct color_blending_operation
	{
		add,
		subtract,
		reverse_subtract,
		min,
		max
	};

	/** Different factors for color blending operations */
	enum struct blending_factor
	{
		zero,
		one,
		source_color,
		one_minus_source_color,
		destination_color,
		one_minus_destination_color,
		source_alpha,
		one_minus_source_alpha,
		destination_alpha,
		one_minus_destination_alpha,
		constant_color,
		one_minus_constant_color,
		constant_alpha,
		one_minus_constant_alpha,
		source_alpha_saturate
	};

	/** Different types operation types for `color_blending_mode::logic_operation` mode */
	enum struct blending_logic_operation
	{
		clear,
		and,
		and_reverse,
		copy,
		and_inverted,
		no_op,
		xor,
		or,
		nor,
		equivalent,
		invert,
		or_reverse,
		copy_inverted,
		or_inverted,
		nand,
		set
	};

	/** Enable or disable color blending and set blend modes */
	struct color_blending
	{
		struct blend_attachment
		{
			bool mBlendingEnabled;
			color_channel mAffectedColorChannels;
			
			color_blending_operation mColorBlendOperation;
			blending_factor mColorSourceFactor;
			blending_factor mColorDestinationFactor;

			color_blending_operation mAlphaBlendOperation;
			blending_factor mAlphaSourceFactor;
			blending_factor mAlphaDestinationFactor;
		};

		static color_blending config_alpha_blending(); // TODO: dieses!
		static color_blending config_additive_blending();

		color_blending_mode mMode;
		blending_logic_operation mLogicOperation;
		glm::vec4 mBlendConstants;
		std::vector<blend_attachment> mBlendAttachments;
	};
	
	/** Pipeline configuration data: BIG GRAPHICS PIPELINE CONFIG STRUCT */
	struct graphics_pipeline_config
	{
		graphics_pipeline_config();
		graphics_pipeline_config(graphics_pipeline_config&&) = default;
		graphics_pipeline_config(const graphics_pipeline_config&) = default;
		graphics_pipeline_config& operator=(graphics_pipeline_config&&) = default;
		graphics_pipeline_config& operator=(const graphics_pipeline_config&) = default;
		~graphics_pipeline_config() = default;

		pipeline_settings mPipelineSettings;
		std::vector<input_binding_location_data> mInputBindingLocations;
		std::vector<shader_info> mShaderInfos;
		std::vector<binding_data> mResourceBindings;
		depth_test mDepthTestConfig;
		depth_write mDepthWriteConfig;
		std::vector<viewport_depth_scissors_config> mViewportDepthConfig;
		culling_mode mCullingMode;
		front_face mFrontFaceWindingOrder;
		polygon_drawing mPolygonDrawingModeAndConfig;
		rasterizer_geometry_mode mRasterizerGeometryMode;
		depth_settings mDepthSettings;
	};



}