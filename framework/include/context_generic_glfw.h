#pragma once
#include "context_generic_glfw_types.h"

namespace cgb
{
	// ============================= forward declarations ===========================
	class window;
	class input_buffer;
	enum struct key_code;

	// =============================== type aliases =================================
	using window_ptr = std::unique_ptr<window>;
	using dispatcher_action = void(void);

	// =========================== GLFW (PARTIAL) CONTEXT ===========================
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
		
		/** Prepares a new window */
		window* prepare_window();

		/** Close the given window, cleanup the resources */
		void close_window(window& wnd);

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

		/** Returns the first window which has been created
		 */
		window* main_window() const;

		/** Returns the window which matches the given name, if it is present in the composition.
		 *	@param	pName	Name of the window
		 *  @return	Pointer to the window with the given name or nullptr if no window matches
		 */
		window* window_by_title(const std::string& pTitle) const;

		/** Returns the window which matches the given id, if it is present in the composition.
		 *	@param	pId		Id of the window
		 *  @return	Pointer to the window with the given name or nullptr if no window matches
		 */
		window* window_by_id(uint32_t pId) const;

		/** Select multiple windows and return a vector of pointers to them.
		 *  Example: To select all windows, pass the lambda [](auto* w){ return true; }
		 */
		template <typename T>
		std::vector<window*> select_windows(T selector)
		{
			std::vector<window*> results;
			for (auto& wnd : mWindows) {
				auto wnd_ptr = wnd.get();
				if (selector(wnd_ptr)) {
					results.push_back(wnd_ptr);
				}
			}
			return results;
		}

		/** Returns the window which is currently in focus, i.e. this is also
		 *	the window which is affected by all mouse cursor input interaction.
		 */
		window* window_in_focus() const { return mWindowInFocus; }

		/** With this context, all windows share the same graphics-context, this 
		 *	method can be used to get a window to share the context with.
		 */
		GLFWwindow* get_window_for_shared_context();

		/** Returns true if the calling thread is the main thread, false otherwise. */
		static bool are_we_on_the_main_thread();

		/**	Dispatch an action to the main thread and have it executed there.
		 *	@param	pAction	The action to execute on the main thread.
		 */
		void dispatch_to_main_thread(std::function<dispatcher_action> pAction);

		/** Works off all elements in the mDispatchQueue
		 */
		void work_off_all_pending_main_thread_actions();

	protected:
		static void glfw_error_callback(int error, const char* description);
		static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
		static void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
		static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void window_focus_callback(GLFWwindow* window, int focused);

		std::vector<window_ptr> mWindows;
		static window* mWindowInFocus;
		bool mInitialized;

		static std::mutex sInputMutex;
		static input_buffer* sTargetInputBuffer;
		static std::array<key_code, GLFW_KEY_LAST + 1> sGlfwToKeyMapping;

		static std::thread::id sMainThreadId;
		static std::mutex sDispatchMutex;
		std::vector<std::function<dispatcher_action>> mDispatchQueue;
	};
}
