#include "context_generic_glfw.h"
#include "window.h"
#include "input_buffer.h"

namespace cgb
{
	std::mutex generic_glfw::sInputMutex;
	input_buffer* generic_glfw::sTargetInputBuffer(nullptr);
	std::array<key_code, GLFW_KEY_LAST + 1> generic_glfw::sGlfwToKeyMapping{};

	generic_glfw::generic_glfw()
		: mInitialized(false)

	{
		LOG_VERBOSE("Creating GLFW context...");
	
		// Setting an error callback
		glfwSetErrorCallback(glfw_error_callback);
		
		// Initializing GLFW
		if (GLFW_TRUE == glfwInit())
		{
			mInitialized = true;
		}
		else
		{
			LOG_ERROR("glfwInit failed");
		}

		for (auto& i : sGlfwToKeyMapping)
		{
			i = key_code::unknown;
		}
		sGlfwToKeyMapping[0] = key_code::unknown;
		sGlfwToKeyMapping[GLFW_KEY_SPACE] = key_code::space;
		sGlfwToKeyMapping[GLFW_KEY_APOSTROPHE] = key_code::apostrophe;
		sGlfwToKeyMapping[GLFW_KEY_COMMA] = key_code::comma;
		sGlfwToKeyMapping[GLFW_KEY_MINUS] = key_code::minus;
		sGlfwToKeyMapping[GLFW_KEY_PERIOD] = key_code::period;
		sGlfwToKeyMapping[GLFW_KEY_SLASH] = key_code::slash;
		sGlfwToKeyMapping[GLFW_KEY_0] = key_code::num0;
		sGlfwToKeyMapping[GLFW_KEY_1] = key_code::num1;
		sGlfwToKeyMapping[GLFW_KEY_2] = key_code::num2;
		sGlfwToKeyMapping[GLFW_KEY_3] = key_code::num3;
		sGlfwToKeyMapping[GLFW_KEY_4] = key_code::num4;
		sGlfwToKeyMapping[GLFW_KEY_5] = key_code::num5;
		sGlfwToKeyMapping[GLFW_KEY_6] = key_code::num6;
		sGlfwToKeyMapping[GLFW_KEY_7] = key_code::num7;
		sGlfwToKeyMapping[GLFW_KEY_8] = key_code::num8;
		sGlfwToKeyMapping[GLFW_KEY_9] = key_code::num9;
		sGlfwToKeyMapping[GLFW_KEY_SEMICOLON] = key_code::semicolon;
		sGlfwToKeyMapping[GLFW_KEY_EQUAL] = key_code::equal;
		sGlfwToKeyMapping[GLFW_KEY_A] = key_code::a;
		sGlfwToKeyMapping[GLFW_KEY_B] = key_code::b;
		sGlfwToKeyMapping[GLFW_KEY_C] = key_code::c;
		sGlfwToKeyMapping[GLFW_KEY_D] = key_code::d;
		sGlfwToKeyMapping[GLFW_KEY_E] = key_code::e;
		sGlfwToKeyMapping[GLFW_KEY_F] = key_code::f;
		sGlfwToKeyMapping[GLFW_KEY_G] = key_code::g;
		sGlfwToKeyMapping[GLFW_KEY_H] = key_code::h;
		sGlfwToKeyMapping[GLFW_KEY_I] = key_code::i;
		sGlfwToKeyMapping[GLFW_KEY_J] = key_code::j;
		sGlfwToKeyMapping[GLFW_KEY_K] = key_code::k;
		sGlfwToKeyMapping[GLFW_KEY_L] = key_code::l;
		sGlfwToKeyMapping[GLFW_KEY_M] = key_code::m;
		sGlfwToKeyMapping[GLFW_KEY_N] = key_code::n;
		sGlfwToKeyMapping[GLFW_KEY_O] = key_code::o;
		sGlfwToKeyMapping[GLFW_KEY_P] = key_code::p;
		sGlfwToKeyMapping[GLFW_KEY_Q] = key_code::q;
		sGlfwToKeyMapping[GLFW_KEY_R] = key_code::r;
		sGlfwToKeyMapping[GLFW_KEY_S] = key_code::s;
		sGlfwToKeyMapping[GLFW_KEY_T] = key_code::t;
		sGlfwToKeyMapping[GLFW_KEY_U] = key_code::u;
		sGlfwToKeyMapping[GLFW_KEY_V] = key_code::v;
		sGlfwToKeyMapping[GLFW_KEY_W] = key_code::w;
		sGlfwToKeyMapping[GLFW_KEY_X] = key_code::x;
		sGlfwToKeyMapping[GLFW_KEY_Y] = key_code::y;
		sGlfwToKeyMapping[GLFW_KEY_Z] = key_code::z;
		sGlfwToKeyMapping[GLFW_KEY_LEFT_BRACKET] = key_code::left_bracket;
		sGlfwToKeyMapping[GLFW_KEY_BACKSLASH] = key_code::backslash;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT_BRACKET] = key_code::right_bracket;
		sGlfwToKeyMapping[GLFW_KEY_GRAVE_ACCENT] = key_code::grave_accent;
		sGlfwToKeyMapping[GLFW_KEY_WORLD_1] = key_code::world_1;
		sGlfwToKeyMapping[GLFW_KEY_WORLD_2] = key_code::world_2;
		sGlfwToKeyMapping[GLFW_KEY_ESCAPE] = key_code::escape;
		sGlfwToKeyMapping[GLFW_KEY_ENTER] = key_code::enter;
		sGlfwToKeyMapping[GLFW_KEY_TAB] = key_code::tab;
		sGlfwToKeyMapping[GLFW_KEY_BACKSPACE] = key_code::backspace;
		sGlfwToKeyMapping[GLFW_KEY_INSERT] = key_code::insert;
		sGlfwToKeyMapping[GLFW_KEY_DELETE] = key_code::del;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT] = key_code::right;
		sGlfwToKeyMapping[GLFW_KEY_LEFT] = key_code::left;
		sGlfwToKeyMapping[GLFW_KEY_DOWN] = key_code::down;
		sGlfwToKeyMapping[GLFW_KEY_UP] = key_code::up;
		sGlfwToKeyMapping[GLFW_KEY_PAGE_UP] = key_code::page_up;
		sGlfwToKeyMapping[GLFW_KEY_PAGE_DOWN] = key_code::page_down;
		sGlfwToKeyMapping[GLFW_KEY_HOME] = key_code::home;
		sGlfwToKeyMapping[GLFW_KEY_END] = key_code::end;
		sGlfwToKeyMapping[GLFW_KEY_CAPS_LOCK] = key_code::caps_lock;
		sGlfwToKeyMapping[GLFW_KEY_SCROLL_LOCK] = key_code::scroll_lock;
		sGlfwToKeyMapping[GLFW_KEY_NUM_LOCK] = key_code::num_lock;
		sGlfwToKeyMapping[GLFW_KEY_PRINT_SCREEN] = key_code::print_screen;
		sGlfwToKeyMapping[GLFW_KEY_PAUSE] = key_code::pause;
		sGlfwToKeyMapping[GLFW_KEY_F1] = key_code::f1;
		sGlfwToKeyMapping[GLFW_KEY_F2] = key_code::f2;
		sGlfwToKeyMapping[GLFW_KEY_F3] = key_code::f3;
		sGlfwToKeyMapping[GLFW_KEY_F4] = key_code::f4;
		sGlfwToKeyMapping[GLFW_KEY_F5] = key_code::f5;
		sGlfwToKeyMapping[GLFW_KEY_F6] = key_code::f6;
		sGlfwToKeyMapping[GLFW_KEY_F7] = key_code::f7;
		sGlfwToKeyMapping[GLFW_KEY_F8] = key_code::f8;
		sGlfwToKeyMapping[GLFW_KEY_F9] = key_code::f9;
		sGlfwToKeyMapping[GLFW_KEY_F10] = key_code::f10;
		sGlfwToKeyMapping[GLFW_KEY_F11] = key_code::f11;
		sGlfwToKeyMapping[GLFW_KEY_F12] = key_code::f12;
		sGlfwToKeyMapping[GLFW_KEY_F13] = key_code::f13;
		sGlfwToKeyMapping[GLFW_KEY_F14] = key_code::f14;
		sGlfwToKeyMapping[GLFW_KEY_F15] = key_code::f15;
		sGlfwToKeyMapping[GLFW_KEY_F16] = key_code::f16;
		sGlfwToKeyMapping[GLFW_KEY_F17] = key_code::f17;
		sGlfwToKeyMapping[GLFW_KEY_F18] = key_code::f18;
		sGlfwToKeyMapping[GLFW_KEY_F19] = key_code::f19;
		sGlfwToKeyMapping[GLFW_KEY_F20] = key_code::f20;
		sGlfwToKeyMapping[GLFW_KEY_F21] = key_code::f21;
		sGlfwToKeyMapping[GLFW_KEY_F22] = key_code::f22;
		sGlfwToKeyMapping[GLFW_KEY_F23] = key_code::f23;
		sGlfwToKeyMapping[GLFW_KEY_F24] = key_code::f24;
		sGlfwToKeyMapping[GLFW_KEY_F25] = key_code::f25;
		sGlfwToKeyMapping[GLFW_KEY_KP_0] = key_code::numpad_0;
		sGlfwToKeyMapping[GLFW_KEY_KP_1] = key_code::numpad_1;
		sGlfwToKeyMapping[GLFW_KEY_KP_2] = key_code::numpad_2;
		sGlfwToKeyMapping[GLFW_KEY_KP_3] = key_code::numpad_3;
		sGlfwToKeyMapping[GLFW_KEY_KP_4] = key_code::numpad_4;
		sGlfwToKeyMapping[GLFW_KEY_KP_5] = key_code::numpad_5;
		sGlfwToKeyMapping[GLFW_KEY_KP_6] = key_code::numpad_6;
		sGlfwToKeyMapping[GLFW_KEY_KP_7] = key_code::numpad_7;
		sGlfwToKeyMapping[GLFW_KEY_KP_8] = key_code::numpad_8;
		sGlfwToKeyMapping[GLFW_KEY_KP_9] = key_code::numpad_9;
		sGlfwToKeyMapping[GLFW_KEY_KP_DECIMAL] = key_code::numpad_decimal;
		sGlfwToKeyMapping[GLFW_KEY_KP_DIVIDE] = key_code::numpad_divide;
		sGlfwToKeyMapping[GLFW_KEY_KP_MULTIPLY] = key_code::numpad_multiply;
		sGlfwToKeyMapping[GLFW_KEY_KP_SUBTRACT] = key_code::numpad_subtract;
		sGlfwToKeyMapping[GLFW_KEY_KP_ADD] = key_code::numpad_add;
		sGlfwToKeyMapping[GLFW_KEY_KP_ENTER] = key_code::numpad_enter;
		sGlfwToKeyMapping[GLFW_KEY_KP_EQUAL] = key_code::numpad_equal;
		sGlfwToKeyMapping[GLFW_KEY_LEFT_SHIFT] = key_code::left_shift;
		sGlfwToKeyMapping[GLFW_KEY_LEFT_CONTROL] = key_code::left_control;
		sGlfwToKeyMapping[GLFW_KEY_LEFT_ALT] = key_code::left_alt;
		sGlfwToKeyMapping[GLFW_KEY_LEFT_SUPER] = key_code::left_super;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT_SHIFT] = key_code::right_shift;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT_CONTROL] = key_code::right_control;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT_ALT] = key_code::right_alt;
		sGlfwToKeyMapping[GLFW_KEY_RIGHT_SUPER] = key_code::right_super;
		sGlfwToKeyMapping[GLFW_KEY_MENU] = key_code::menu;
	}

	generic_glfw::~generic_glfw()
	{
		if (mInitialized)
		{
			glfwTerminate();
			// context has been desroyed by glfwTerminate
			mInitialized = false;
		}
	}

	generic_glfw::operator bool() const
	{
		return mInitialized;
	}

	window* generic_glfw::create_window(const window_params& pParams)
	{
		// Share a context or let GLFW create a new one
		GLFWwindow* contextToUse = nullptr; 
		if (pParams.mSharedContext)
		{
			contextToUse = pParams.mSharedContext->mWindowHandle;
		}

		// Determine the resolution or have it set to the default of 1600x900  (because, why not?!)
		int width = 1600;
		int height = 900;
		if (pParams.mInitialWidth && pParams.mInitialHeight)
		{
			width = *pParams.mInitialWidth;
			height = *pParams.mInitialHeight;
		}
		else
		{
			if (pParams.mMonitor)
			{
				const auto* mode = glfwGetVideoMode(pParams.mMonitor->mMonitorHandle);
				width = mode->width;
				height = mode->height;
			}
		}

		// Before creating the window, set some config-parameters
		if (pParams.mEnableSrgbFramebuffer) 
		{
			glfwWindowHint(GLFW_SRGB_CAPABLE, *pParams.mEnableSrgbFramebuffer ? GLFW_TRUE : GLFW_FALSE); 
		}
		glfwWindowHint(GLFW_DOUBLEBUFFER, false == pParams.mEnableDoublebuffering || *pParams.mEnableDoublebuffering ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_SAMPLES, pParams.mNumberOfSamplesForMSAA ? *pParams.mNumberOfSamplesForMSAA : 1);
		GLFWwindow* handle = glfwCreateWindow(
			width,
			height,
			pParams.mWindowTitle.empty() ? "cg_base: GLFW Window" : pParams.mWindowTitle.c_str(),
			static_cast<bool>(pParams.mMonitor) ? pParams.mMonitor->mMonitorHandle : nullptr,
			contextToUse); // TODO: make configurable
		
		if (!handle)
		{
			throw std::runtime_error("glfwCreateWindow failed"); 
		}

		mWindows.push_back(std::make_unique<window>(window_handle{ handle }));
		// TODO: schau mol:
		return mWindows[mWindows.size() - 1].get();
	}

	void generic_glfw::close_window(window& wnd)
	{
		if (!wnd.handle())
		{
			LOG_WARNING("The passed window has no valid handle. Has it already been destroyed?");
			return;
		}

		glfwDestroyWindow(wnd.handle()->mWindowHandle);
		wnd.mHandle = std::nullopt;
	}

	void generic_glfw::enable_vsync(bool enable)
	{
		glfwSwapInterval(enable ? 1 : 0);
	}

	double generic_glfw::get_time()
	{
		return glfwGetTime();
	}

	void generic_glfw::glfw_error_callback(int error, const char* description)
	{
		LOG_ERROR(fmt::format("GLFW-Error: hex[0x{0:x}] int[{0}] description[{1}]", error, description));
	}

	void generic_glfw::start_receiving_input_from_window(const window& pWindow, input_buffer& pInputBuffer)
	{
		glfwSetMouseButtonCallback(pWindow.handle()->mWindowHandle, glfw_mouse_button_callback);
		glfwSetCursorPosCallback(pWindow.handle()->mWindowHandle, glfw_cursor_pos_callback);
		glfwSetScrollCallback(pWindow.handle()->mWindowHandle, glfw_scroll_callback);
		glfwSetKeyCallback(pWindow.handle()->mWindowHandle, glfw_key_callback);
		sTargetInputBuffer = &pInputBuffer;
	}

	void generic_glfw::stop_receiving_input_from_window(const window& pWindow)
	{
		glfwSetMouseButtonCallback(pWindow.handle()->mWindowHandle, nullptr);
	}

	void generic_glfw::change_target_input_buffer(input_buffer& pInputBuffer)
	{
		std::lock_guard<std::mutex> lock(sInputMutex);
		sTargetInputBuffer = &pInputBuffer;
	}

	glm::dvec2 generic_glfw::cursor_position(const window& pWindow)
	{
		std::lock_guard<std::mutex> lock(sInputMutex);
		glm::dvec2 cursorPos;
		assert(pWindow.handle() != std::nullopt);
		glfwGetCursorPos(pWindow.handle()->mWindowHandle, &cursorPos[0], &cursorPos[1]);
		return cursorPos;
	}

	void generic_glfw::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		assert(sTargetInputBuffer);
		std::lock_guard<std::mutex> lock(sInputMutex);
		button = glm::clamp(button, 0, 7);
		switch (action)
		{
		case GLFW_PRESS:
			sTargetInputBuffer->mMouseKeys[button] |= key_state::pressed;
			sTargetInputBuffer->mMouseKeys[button] |= key_state::down;
			break;
		case GLFW_RELEASE:
			sTargetInputBuffer->mMouseKeys[button] |= key_state::released;
			break;
		case GLFW_REPEAT:
			sTargetInputBuffer->mMouseKeys[button] |= key_state::down;
			break;
		}
	}

	void generic_glfw::glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
	{
		assert(sTargetInputBuffer);
		std::lock_guard<std::mutex> lock(sInputMutex);
		sTargetInputBuffer->mCursorPosition = glm::dvec2(xpos, ypos);
	}

	void generic_glfw::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		assert(sTargetInputBuffer);
		std::lock_guard<std::mutex> lock(sInputMutex);
		sTargetInputBuffer->mScrollPosition += glm::dvec2(xoffset, yoffset);
	}

	void generic_glfw::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		assert(sTargetInputBuffer);
		std::lock_guard<std::mutex> lock(sInputMutex);

		// TODO: Do something with the window-parameter? Or is it okay?

		key = glm::clamp(key, 0, GLFW_KEY_LAST);
		switch (action)
		{
		case GLFW_PRESS:
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(sGlfwToKeyMapping[key])] |= key_state::pressed;
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(sGlfwToKeyMapping[key])] |= key_state::down;
			break;
		case GLFW_RELEASE:
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(sGlfwToKeyMapping[key])] |= key_state::released;
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(sGlfwToKeyMapping[key])] &= ~key_state::down;
			break;
		case GLFW_REPEAT:
			// just ignore
			break;
		}
		
	}

}
