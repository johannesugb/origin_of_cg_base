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
}