#pragma once

// ------ commonly used stuff from the standard library ------
#include <vector>
#include <array>
#include <string>
#include <exception>
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

#include <stdio.h>

// ----------------------- externals -----------------------
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/anim.h>

#include <imgui.h>

// Note: GLM is getting included in the context-headers.

// -------------------- windows include ---------------------
#ifdef _WIN32
// Include that after the assimp includes due to some conflict
// with the global scope operator :: in material.inl
#define NOMINMAX
#include <windows.h>
#endif

// -------------------- CG-Base includes --------------------
#include "context_types.h"
#include "context.h"
#include "string_utils.h"
#include "log.h"
#include "various_utils.h"
#include "math_utils.h"
#include "key_code.h"
#include "key_state.h"
#include "window.h"
#include "timer_interface.h"
#include "timer_frame_type.h"
#include "fixed_update_timer.h"
#include "varying_update_only_timer.h"
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
