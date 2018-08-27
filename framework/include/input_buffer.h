#pragma once

namespace cgb
{
	/** \brief Contains all the input data of a frame
	 *
	 *	This structure will be filled during a frame, so that it contains
	 *	the input of the current/last/whichever frame.
	 */
	struct input_buffer
	{
		/** Keyboard button states */
		std::array<key_state, static_cast<size_t>(key_code::max_value)> mKeyboardKeys;

		/** Position of the mouse cursor */
		glm::dvec2 mCursorPosition;

		/** Mouse button states */
		std::array<key_state, 5> mMouseKeys;

		/** Scrolling wheel position data */
		glm::dvec2 mScrollPosition;
	};
}
