#include "input_buffer.h"

namespace cgb
{
	void input_buffer::reset(std::optional<window> pWindow)
	{
		std::fill(std::begin(mKeyboardKeys), std::end(mKeyboardKeys), key_state::none);
		if (pWindow)
		{
			mCursorPosition = context().cursor_position(*pWindow);
		}
		else
		{
			mCursorPosition = {0.0, 0.0};
		}
		std::fill(std::begin(mMouseKeys), std::end(mMouseKeys), key_state::none);
		mScrollPosition = { 0.0, 0.0 };
	}

	void input_buffer::prepare_for_next_frame(const input_buffer& pPreviousFrame, std::optional<window> pWindow)
	{
		for (auto i = 0; i < mKeyboardKeys.size(); ++i)
		{
			mKeyboardKeys[i] = (pPreviousFrame.mKeyboardKeys[i] & key_state::down);
		}
		if (pWindow)
		{
			mCursorPosition = context().cursor_position(*pWindow);
		}
		else
		{
			mCursorPosition = { 0.0, 0.0 };
		}
		for (auto i = 0; i < mMouseKeys.size(); ++i)
		{
			mMouseKeys[i] = (pPreviousFrame.mMouseKeys[i] & key_state::down);
		}
		mScrollPosition = { 0.0, 0.0 };
	}

	bool input_buffer::key_pressed(key_code pKey)
	{
		return (mKeyboardKeys[static_cast<size_t>(pKey)] & key_state::pressed) != key_state::none;
	}

	bool input_buffer::key_released(key_code pKey)
	{
		return (mKeyboardKeys[static_cast<size_t>(pKey)] & key_state::released) != key_state::none;
	}

	bool input_buffer::key_down(key_code pKey)
	{
		return (mKeyboardKeys[static_cast<size_t>(pKey)] & key_state::down) != key_state::none;
	}

	bool input_buffer::mouse_button_pressed(uint8_t pButtonIndex)
	{
		return (mMouseKeys[static_cast<size_t>(pButtonIndex)] & key_state::pressed) != key_state::none;
	}

	bool input_buffer::mouse_button_released(uint8_t pButtonIndex)
	{
		return (mMouseKeys[static_cast<size_t>(pButtonIndex)] & key_state::released) != key_state::none;
	}

	bool input_buffer::mouse_button_down(uint8_t pButtonIndex)
	{
		return (mMouseKeys[static_cast<size_t>(pButtonIndex)] & key_state::down) != key_state::none;
	}

	const glm::dvec2& input_buffer::cursor_position()
	{
		return mCursorPosition;
	}

	glm::dvec2 input_buffer::cursor_position(const window& pWindow)
	{
		return context().cursor_position(pWindow);
	}

	const glm::dvec2& input_buffer::scroll_delta()
	{
		return mScrollPosition;
	}
}