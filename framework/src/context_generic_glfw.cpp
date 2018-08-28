#if defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)
#include "context_generic_glfw.h"
#include "window.h"
#include "input_buffer.h"

namespace cgb
{
	input_buffer* generic_glfw::sTargetInputBuffer(nullptr);

	generic_glfw::generic_glfw() :
		mInitialized(false),
		mFirstWindow(nullptr)
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
	}

	generic_glfw::~generic_glfw()
	{
		if (mInitialized)
		{
			glfwTerminate();
			// context has been desroyed by glfwTerminate
			mFirstWindow = nullptr;
			mInitialized = false;
		}
	}

	generic_glfw::operator bool() const
	{
		return mInitialized;
	}

	window generic_glfw::create_window()
	{
		// For the first window, let GLFW create a new context...
		GLFWwindow* contextToUse = nullptr; 
		if (mFirstWindow)
		{
			// ...for all further windows, share the context
			contextToUse = mFirstWindow; 

			// TODO: what happens to the context and to the other windows, if the first window is closed?
		}

		GLFWwindow* handle = glfwCreateWindow(
			800, // TODO: make width configurable
			600, // TODO: make height configurable
			"TODO: make configurable",
			nullptr, // TODO: make Monitor configurable
			contextToUse); // TODO: make configurable
		
		if (!handle)
		{
			throw std::runtime_error("glfwCreateWindow failed"); 
		}

		if (!mFirstWindow)
		{
			// If context has been newly created in the current call to create_window, 
			// 1) make the newly created context current and
			// 2) use the extension loader to get the proc-addresses (which needs an active context)
			glfwMakeContextCurrent(handle);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			mFirstWindow = handle; // don't do this again next time
			// By (current) design, all windows share the same context.
			// TODO: Think about supporting different contexts somewhen in the future.
		}

		return window(window_handle{ handle });
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

	void generic_glfw::start_receiving_input_from_window(const window& pWindow, input_buffer& pInputBuffer)
	{
		glfwSetMouseButtonCallback(pWindow.handle()->mWindowHandle, glfw_mouse_button_callback);
		glfwSetCursorPosCallback(pWindow.handle()->mWindowHandle, glfw_cursor_pos_callback);
		glfwSetScrollCallback(pWindow.handle()->mWindowHandle, glfw_scroll_callback);
		glfwSetKeyCallback(pWindow.handle()->mWindowHandle, glfw_key_callback);
		sTargetInputBuffer = &pInputBuffer;
	}

	void generic_glfw::change_target_input_buffer(input_buffer& pInputBuffer)
	{
		sTargetInputBuffer = &pInputBuffer;
	}

	void generic_glfw::stop_receiving_input_from_window(const window& pWindow)
	{
		glfwSetMouseButtonCallback(pWindow.handle()->mWindowHandle, nullptr);
	}

	glm::dvec2 generic_glfw::cursor_position(const window& pWindow)
	{
		glm::dvec2 cursorPos;
		assert(pWindow.handle() != std::nullopt);
		glfwGetCursorPos(pWindow.handle()->mWindowHandle, &cursorPos[0], &cursorPos[1]);
		return cursorPos;
	}

	void generic_glfw::glfw_error_callback(int error, const char* description)
	{
		LOG_ERROR("GLFW-Error: hex[0x%x] int[%d] description[%s]", error, error, description);
		LOG_ERROR("GLFW-Error: hex[0x%x] int[%d] description[%s]", error, error, description);
	}

	void generic_glfw::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		assert(sTargetInputBuffer);
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
		sTargetInputBuffer->mCursorPosition = glm::dvec2(xpos, ypos);
	}

	void generic_glfw::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		assert(sTargetInputBuffer);
		sTargetInputBuffer->mScrollPosition = glm::dvec2(xoffset, yoffset);
	}

	void generic_glfw::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		assert(sTargetInputBuffer);
		// TODO: Do something with the window-parameter? Or is it okay?
		static std::array<key_code, GLFW_KEY_LAST+1> button_mapping = []() {
			std::array<key_code, GLFW_KEY_LAST+1> glfw_mapping;
			for (auto& i : glfw_mapping)
			{
				i = key_code::unknown;
			}
			glfw_mapping[GLFW_KEY_UNKNOWN] = key_code::unknown;
			glfw_mapping[GLFW_KEY_SPACE] = key_code::space;
			glfw_mapping[GLFW_KEY_APOSTROPHE] = key_code::apostrophe;
			glfw_mapping[GLFW_KEY_COMMA] = key_code::comma;
			glfw_mapping[GLFW_KEY_MINUS] = key_code::minus;
			glfw_mapping[GLFW_KEY_PERIOD] = key_code::period;
			glfw_mapping[GLFW_KEY_SLASH] = key_code::slash;
			glfw_mapping[GLFW_KEY_0] = key_code::num0;
			glfw_mapping[GLFW_KEY_1] = key_code::num1;
			glfw_mapping[GLFW_KEY_2] = key_code::num2;
			glfw_mapping[GLFW_KEY_3] = key_code::num3;
			glfw_mapping[GLFW_KEY_4] = key_code::num4;
			glfw_mapping[GLFW_KEY_5] = key_code::num5;
			glfw_mapping[GLFW_KEY_6] = key_code::num6;
			glfw_mapping[GLFW_KEY_7] = key_code::num7;
			glfw_mapping[GLFW_KEY_8] = key_code::num8;
			glfw_mapping[GLFW_KEY_9] = key_code::num9;
			glfw_mapping[GLFW_KEY_SEMICOLON] = key_code::semicolon;
			glfw_mapping[GLFW_KEY_EQUAL] = key_code::equal;
			glfw_mapping[GLFW_KEY_A] = key_code::a;
			glfw_mapping[GLFW_KEY_B] = key_code::b;
			glfw_mapping[GLFW_KEY_C] = key_code::c;
			glfw_mapping[GLFW_KEY_D] = key_code::d;
			glfw_mapping[GLFW_KEY_E] = key_code::e;
			glfw_mapping[GLFW_KEY_F] = key_code::f;
			glfw_mapping[GLFW_KEY_G] = key_code::g;
			glfw_mapping[GLFW_KEY_H] = key_code::h;
			glfw_mapping[GLFW_KEY_I] = key_code::i;
			glfw_mapping[GLFW_KEY_J] = key_code::j;
			glfw_mapping[GLFW_KEY_K] = key_code::k;
			glfw_mapping[GLFW_KEY_L] = key_code::l;
			glfw_mapping[GLFW_KEY_M] = key_code::m;
			glfw_mapping[GLFW_KEY_N] = key_code::n;
			glfw_mapping[GLFW_KEY_O] = key_code::o;
			glfw_mapping[GLFW_KEY_P] = key_code::p;
			glfw_mapping[GLFW_KEY_Q] = key_code::q;
			glfw_mapping[GLFW_KEY_R] = key_code::r;
			glfw_mapping[GLFW_KEY_S] = key_code::s;
			glfw_mapping[GLFW_KEY_T] = key_code::t;
			glfw_mapping[GLFW_KEY_U] = key_code::u;
			glfw_mapping[GLFW_KEY_V] = key_code::v;
			glfw_mapping[GLFW_KEY_W] = key_code::w;
			glfw_mapping[GLFW_KEY_X] = key_code::x;
			glfw_mapping[GLFW_KEY_Y] = key_code::y;
			glfw_mapping[GLFW_KEY_Z] = key_code::z;
			glfw_mapping[GLFW_KEY_LEFT_BRACKET] = key_code::left_bracket;
			glfw_mapping[GLFW_KEY_BACKSLASH] = key_code::backslash;
			glfw_mapping[GLFW_KEY_RIGHT_BRACKET] = key_code::right_bracket;
			glfw_mapping[GLFW_KEY_GRAVE_ACCENT] = key_code::grave_accent;
			glfw_mapping[GLFW_KEY_WORLD_1] = key_code::world_1;
			glfw_mapping[GLFW_KEY_WORLD_2] = key_code::world_2;
			glfw_mapping[GLFW_KEY_ESCAPE] = key_code::escape;
			glfw_mapping[GLFW_KEY_ENTER] = key_code::enter;
			glfw_mapping[GLFW_KEY_TAB] = key_code::tab;
			glfw_mapping[GLFW_KEY_BACKSPACE] = key_code::backspace;
			glfw_mapping[GLFW_KEY_INSERT] = key_code::insert;
			glfw_mapping[GLFW_KEY_DELETE] = key_code::del;
			glfw_mapping[GLFW_KEY_RIGHT] = key_code::right;
			glfw_mapping[GLFW_KEY_LEFT] = key_code::left;
			glfw_mapping[GLFW_KEY_DOWN] = key_code::down;
			glfw_mapping[GLFW_KEY_UP] = key_code::up;
			glfw_mapping[GLFW_KEY_PAGE_UP] = key_code::page_up;
			glfw_mapping[GLFW_KEY_PAGE_DOWN] = key_code::page_down;
			glfw_mapping[GLFW_KEY_HOME] = key_code::home;
			glfw_mapping[GLFW_KEY_END] = key_code::end;
			glfw_mapping[GLFW_KEY_CAPS_LOCK] = key_code::caps_lock;
			glfw_mapping[GLFW_KEY_SCROLL_LOCK] = key_code::scroll_lock;
			glfw_mapping[GLFW_KEY_NUM_LOCK] = key_code::num_lock;
			glfw_mapping[GLFW_KEY_PRINT_SCREEN] = key_code::print_screen;
			glfw_mapping[GLFW_KEY_PAUSE] = key_code::pause;
			glfw_mapping[GLFW_KEY_F1] = key_code::f1;
			glfw_mapping[GLFW_KEY_F2] = key_code::f2;
			glfw_mapping[GLFW_KEY_F3] = key_code::f3;
			glfw_mapping[GLFW_KEY_F4] = key_code::f4;
			glfw_mapping[GLFW_KEY_F5] = key_code::f5;
			glfw_mapping[GLFW_KEY_F6] = key_code::f6;
			glfw_mapping[GLFW_KEY_F7] = key_code::f7;
			glfw_mapping[GLFW_KEY_F8] = key_code::f8;
			glfw_mapping[GLFW_KEY_F9] = key_code::f9;
			glfw_mapping[GLFW_KEY_F10] = key_code::f10;
			glfw_mapping[GLFW_KEY_F11] = key_code::f11;
			glfw_mapping[GLFW_KEY_F12] = key_code::f12;
			glfw_mapping[GLFW_KEY_F13] = key_code::f13;
			glfw_mapping[GLFW_KEY_F14] = key_code::f14;
			glfw_mapping[GLFW_KEY_F15] = key_code::f15;
			glfw_mapping[GLFW_KEY_F16] = key_code::f16;
			glfw_mapping[GLFW_KEY_F17] = key_code::f17;
			glfw_mapping[GLFW_KEY_F18] = key_code::f18;
			glfw_mapping[GLFW_KEY_F19] = key_code::f19;
			glfw_mapping[GLFW_KEY_F20] = key_code::f20;
			glfw_mapping[GLFW_KEY_F21] = key_code::f21;
			glfw_mapping[GLFW_KEY_F22] = key_code::f22;
			glfw_mapping[GLFW_KEY_F23] = key_code::f23;
			glfw_mapping[GLFW_KEY_F24] = key_code::f24;
			glfw_mapping[GLFW_KEY_F25] = key_code::f25;
			glfw_mapping[GLFW_KEY_KP_0] = key_code::numpad_0;
			glfw_mapping[GLFW_KEY_KP_1] = key_code::numpad_1;
			glfw_mapping[GLFW_KEY_KP_2] = key_code::numpad_2;
			glfw_mapping[GLFW_KEY_KP_3] = key_code::numpad_3;
			glfw_mapping[GLFW_KEY_KP_4] = key_code::numpad_4;
			glfw_mapping[GLFW_KEY_KP_5] = key_code::numpad_5;
			glfw_mapping[GLFW_KEY_KP_6] = key_code::numpad_6;
			glfw_mapping[GLFW_KEY_KP_7] = key_code::numpad_7;
			glfw_mapping[GLFW_KEY_KP_8] = key_code::numpad_8;
			glfw_mapping[GLFW_KEY_KP_9] = key_code::numpad_9;
			glfw_mapping[GLFW_KEY_KP_DECIMAL] = key_code::numpad_decimal;
			glfw_mapping[GLFW_KEY_KP_DIVIDE] = key_code::numpad_divide;
			glfw_mapping[GLFW_KEY_KP_MULTIPLY] = key_code::numpad_multiply;
			glfw_mapping[GLFW_KEY_KP_SUBTRACT] = key_code::numpad_subtract;
			glfw_mapping[GLFW_KEY_KP_ADD] = key_code::numpad_add;
			glfw_mapping[GLFW_KEY_KP_ENTER] = key_code::numpad_enter;
			glfw_mapping[GLFW_KEY_KP_EQUAL] = key_code::numpad_equal;
			glfw_mapping[GLFW_KEY_LEFT_SHIFT] = key_code::left_shift;
			glfw_mapping[GLFW_KEY_LEFT_CONTROL] = key_code::left_control;
			glfw_mapping[GLFW_KEY_LEFT_ALT] = key_code::left_alt;
			glfw_mapping[GLFW_KEY_LEFT_SUPER] = key_code::left_super;
			glfw_mapping[GLFW_KEY_RIGHT_SHIFT] = key_code::right_shift;
			glfw_mapping[GLFW_KEY_RIGHT_CONTROL] = key_code::right_control;
			glfw_mapping[GLFW_KEY_RIGHT_ALT] = key_code::right_alt;
			glfw_mapping[GLFW_KEY_RIGHT_SUPER] = key_code::right_super;
			glfw_mapping[GLFW_KEY_MENU] = key_code::menu;
			return glfw_mapping;
		}();
		assert(key <= GLFW_KEY_LAST);
		switch (action)
		{
		case GLFW_PRESS:
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(button_mapping[key])] |= key_state::pressed;
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(button_mapping[key])] |= key_state::down;
			break;
		case GLFW_RELEASE:
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(button_mapping[key])] |= key_state::released;
			break;
		case GLFW_REPEAT:
			sTargetInputBuffer->mKeyboardKeys[static_cast<size_t>(button_mapping[key])] |= key_state::down;
			break;
		}
		
	}

}

#endif // defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)
