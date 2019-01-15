#pragma once

namespace cgb
{
	/** @brief Contains all the input data of a frame
	 *
	 *	This structure will be filled during a frame, so that it contains
	 *	the input of the current/last/whichever frame.
	 */
	class input_buffer
	{
#if defined(USE_OPENGL46_CONTEXT)
		friend class generic_glfw;
		friend class opengl46;
#elif defined(USE_VULKAN_CONTEXT)
		friend class generic_glfw;
		friend class vulkan;
#endif
	public:
		/** Resets all the input values to a state representing no input.
		 *	If a window is passed, the cursor is set to the cursor position
		 *	w.r.t. to that window.
		 *	(This could be useful at the beginning of a frame)
		 */
		void reset(std::optional<window> pWindow = std::nullopt);

		/** Prepares this input buffer for the next frame based on data of
		 *	the previous frame. This means that key-down states are preserved.
		 */
		void prepare_for_next_frame(const input_buffer& pPreviousFrame, window* pWindow = nullptr);

		/** @brief Keyboard key pressed-down?
		 *
		 *	True if the given keyboard key has been pressed down in the 
		 *	current input-frame 
		 */
		bool key_pressed(key_code);

		/** @brief Keyboard key released?
		 *
		 *	True if the given keyboard key has been released in the
		 *	current input-frame
		 */
		bool key_released(key_code);

		/** @brief Keyboard key held down?
		 *
		 *	True if the given keyboard key is (possibly repeatedly) 
		 *	held down in the current input-frame.
		 *	
		 *	\remark If @ref key_pressed is true, key_down will 
		 *	be true either in any case for the given @ref key_code.
		 */
		bool key_down(key_code);

		/** @brief Mouse button pressed-down?
		 *
		 *	True if the mouse button with the given index has been 
		 *	pressed down in the current input-frame
		 */
		bool mouse_button_pressed(uint8_t);

		/** @brief Mouse button released?
		 *
		 *	True if the mouse button with the given index has been
		 *	released in the current input-frame
		 */
		bool mouse_button_released(uint8_t);

		/** @brief Mouse button held down?
		 *
		 *	True if the mouse button with the given index is (possibly 
		 *	repeatedly) held down in the current input-frame
		 */
		bool mouse_button_down(uint8_t);

		/** @brief Cursor position in the last active window
		 *
		 *	Returns the position of the cursor in the window where it 
		 *	last active.
		 */
		const glm::dvec2& cursor_position();

		/** @brief Cursor position in the given window
		 *
		 *	Returns the position of the cursor w.r.t. the given window.
		 */
		glm::dvec2 cursor_position(const window&);

		/** @brief Mouse wheel's scroll delta
		 *
		 *	Returns the accumulated scrolling delta performed with
		 *	the mouse wheel during the current input frame.
		 */
		const glm::dvec2& scroll_delta();

		/** Sets whether or not the cursor should be hidden */
		void set_cursor_hidden(bool pHidden);

		/** Returns if the cursor is hidden or not */
		bool is_cursor_hidden() const;

		/** Sets the cursor position to the given coordinates */
		void set_cursor_position(glm::dvec2 pPosition);

		/** Positions the cursor in the center of the screen */
		void center_cursor_position(window& pWindow);

	private:
		/** Keyboard button states */
		std::array<key_state, static_cast<size_t>(key_code::max_value)> mKeyboardKeys;

		/** Position of the mouse cursor */
		glm::dvec2 mCursorPosition;

		/** Mouse button states */
		std::array<key_state, 8> mMouseKeys;

		/** Scrolling wheel position data */
		glm::dvec2 mScrollPosition;

		/** True if the cursor is hidden, false otherwise */
		bool mCursorHidden;

		/** Has value if the cursor's visibility should be changed */
		std::optional<bool> mSetCursorHidden;

		/** Has value if the cursor's position should be changed */
		std::optional<glm::dvec2> mSetCursorPosition;
	};
}
