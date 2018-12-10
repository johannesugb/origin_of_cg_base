#pragma once
#include "context_generic_glfw_types.h"

namespace cgb
{
	// ---------------- forward declarations ----------------
	class window;
	class input_buffer;
	enum struct key_code;

	/** @brief Provides generic GLFW-specific functionality
	 */
	class generic_glfw
	{
	public:
		/** Initializes GLFW */
		generic_glfw();
		
		/** Cleans up GLFW stuff */
		virtual ~generic_glfw();
		
		/** Evaluates to true if GLFW initialization succeeded  */
		operator bool() const;
		
		/** Creates a new window 
		 */
		window create_window(const window_params&);

		/** Close the given window, cleanup the resources */
		void close_window(window& wnd);

		/** Enable or disable v-sync
		 *  This might work only once! :O See GLFW docu for further details!
		 *  http://www.glfw.org/docs/3.1/group__context.html#ga6d4e0cdf151b5e579bd67f13202994ed
		 *
		 *  @param enable true to enable, false to disable v-sync
		 */
		void enable_vsync(bool enable);

		/** Gets the current system time */
		double get_time();

		/** @brief starts receiving mouse and keyboard input from specified window.
		 *
		 *	@param[in] pWindow The window to receive input from
		 *	@param[ref] pInputBuffer The input buffer to be filled with user input
		 */
		void start_receiving_input_from_window(const window& pWindow, input_buffer& pInputBuffer);

		/** Change the target input buffer to be modified by input events */
		void change_target_input_buffer(input_buffer& pInputBuffer);

		/**	@brief stops receiving mouse and keyboard input from specified window.
		 *
		 *	@param[in] pWindow The window to stop receiving input from
		 */
		void stop_receiving_input_from_window(const window& pWindow);

		/** Get the cursor position w.r.t. the given window */
		glm::dvec2 cursor_position(const window& pWindow);

	protected:
		static void glfw_error_callback(int error, const char* description);
		static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		static void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
		static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

		bool mInitialized;
		static std::mutex sInputMutex;
		static input_buffer* sTargetInputBuffer;
		static std::array<key_code, GLFW_KEY_LAST + 1> sGlfwToKeyMapping;
	};
}
