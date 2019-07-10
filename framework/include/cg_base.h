#pragma once

// ------ commonly used stuff from the standard library ------
#include <vector>
#include <array>
#include <string>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <functional>
#include <memory>
#include <iostream>
#include <ostream>
#include <fstream>
#include <queue>
#include <algorithm>
#include <variant>
#include <iomanip>
#include <optional>
#include <typeinfo>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdlib>

#include <stdio.h>
#include <assert.h>

// ----------------------- externals -----------------------
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/anim.h>

#include <imgui.h>

// GLM: 
#if defined(USE_VULKAN_CONTEXT)
// DEFINES:
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
// The perspective projection matrix generated by GLM will use the OpenGL depth range of -1.0 to 1.0 by default. 
// We need to configure it to use the Vulkan range of 0.0 to 1.0 using the GLM_FORCE_DEPTH_ZERO_TO_ONE definition. [1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/quaternion_exponential.hpp>
#include <glm/ext/quaternion_relational.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/hash.hpp>

// -------------------- windows include ---------------------
#ifdef _WIN32
// Include that after the assimp includes due to some conflict
// with the global scope operator :: in material.inl
#define NOMINMAX
#include <windows.h>
#endif

// -------------------- CG-Base includes --------------------
#include "essential_utils.h"
#include "log.h"
#include "memory_usage.h"
#include "context_state.h"
#include "device_queue_selection_strategy.h"
#include "buffer_data.h"
#include "buffer.h"

#if defined(USE_OPENGL46_CONTEXT)

#include "context_opengl46.h"

#elif defined(USE_VULKAN_CONTEXT)

#include <vulkan/vulkan.hpp>

#include "image_vulkan.h"
#include "context_generic_glfw_types.h"
#include "window_base.h"
#include "context_generic_glfw.h"
#include "image_view_vulkan.h"
#include "sampler_vulkan.h"
#include "image_sampler_vulkan.h"
#include "vulkan_helper_functions.h"
#include "command_buffer_vulkan.h"
#include "command_pool_vulkan.h"
#include "synchronization_vulkan.h"
#include "queue_vulkan.h"
#include "framebuffer_vulkan.h"
#include "window_vulkan.h"
#include "buffer_vulkan.h"
#include "shader_vulkan.h"
#include "descriptor_pool_vulkan.h"
#include "binding_data_vulkan.h"
#include "descriptor_set_vulkan.h"
#include "acceleration_structure_vulkan.h"
#include "pipeline_vulkan.h"
#include "shader_binding_table_vulkan.h"
#include "bindings_vulkan.h"

#include "imgui_impl_vulkan.h"

#include "context_vulkan.h"
#include "context.h"

#include "vulkan_convenience_functions.h"

// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
// [3] Vulkan Tutorial, Images, https://vulkan-tutorial.com/Texture_mapping/Images
// [4] Vulkan Tutorial, Image view and sampler, https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler

#endif


#include "image.h"
#include "shader_type.h"
#include "shader_source_info.h"
#include "string_utils.h"
#include "various_utils.h"
#include "math_utils.h"
#include "key_code.h"
#include "key_state.h"
#include "timer_interface.h"
#include "timer_frame_type.h"
#include "fixed_update_timer.h"
#include "varying_update_timer.h"

#include "input_buffer.h"
#include "composition_interface.h"

namespace cgb
{
#pragma region global data representing the currently active composition
	/**	@brief Get the current timer, which represents the current game-/render-time
	 *	\remark This is just a shortcut to @ref composition_interface::current()->time();
	 */
	inline timer_interface& time()
	{
		return composition_interface::current()->time();
	}

	/** @brief Get the current frame's input data
	 *	\remark This is just a shortcut to @ref composition_interface::current()->input();
	 */
	inline input_buffer& input()
	{
		return composition_interface::current()->input();
	}

	/** @brief Get access to the currently active objects
	 *	\remark This is just a shortcut to @ref *composition_interface::current();
	 */
	inline composition_interface& current_composition()
	{
		return *composition_interface::current();
	}
#pragma endregion 
}

#include "cg_element.h"
#include "sequential_executor.h"
#include "composition.h"

#include "transform.h"
#include "model.h"
#include "camera.h"
#include "quake_camera.h"
