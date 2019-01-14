#include "window.h"

namespace cgb
{
	uint32_t window::mNextWindowId = 0u;

	window::window(window_handle handle) :
		mWindowId(mNextWindowId++),
		mName(),
		mHandle(std::move(handle)),
		mWidth(0),
		mHeight(0),
		mTitle(),
		mMonitor()
	{

	}

	window::~window()
	{
		if (mHandle)
		{
			context().close_window(*this);
			mHandle = std::nullopt;
		}
	}

	window::window(window&& other) noexcept :
		mName(std::move(other.mName)),
		mHandle(std::move(other.mHandle)),
		mWidth(std::move(other.mWidth)),
		mHeight(std::move(other.mHeight)),
		mTitle(std::move(other.mTitle)),
		mMonitor(std::move(other.mMonitor))
	{
		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;
	}

	window& window::operator =(window&& other) noexcept
	{
		mName = std::move(other.mName);
		mHandle = std::move(other.mHandle);
		mWidth = std::move(other.mWidth);
		mHeight = std::move(other.mHeight);
		mTitle = std::move(other.mTitle);
		mMonitor = std::move(other.mMonitor);

		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;

		return *this;
	}

	void window::set_name(std::string pName)
	{
		mName = pName;
	}

	void window::set_resolution(int pWidth, int pHeight)
	{
		mWidth = pWidth;
		mHeight = pHeight;
	}

	void window::set_title(std::string pTitle)
	{
		mTitle = pTitle;
		// TODO: set the title to the *real* window
	}

	void window::change_monitor(std::optional<monitor_handle> pMonitor)
	{
		mMonitor = pMonitor;
		// TODO: assign the window to the monitor
	}

	void window::set_cursor_hidden(bool pHidden)
	{
		context().hide_cursor(*this, pHidden);
	}

	bool window::is_cursor_hidden() const
	{
		return context().is_cursor_hidden(*this);
	}

	void window::set_cursor_position(int pX, int pY)
	{
		context().set_cursor_pos(*this, { static_cast<double>(pX), static_cast<double>(pY) });
	}

	void window::center_cursor_position()
	{
		set_cursor_position(width() / 2, height() / 2);
	}

	glm::ivec2 window::cursor_position()
	{
		auto dPos = context().cursor_position(*this);
		return glm::ivec2(static_cast<int>(dPos[0]), static_cast<int>(dPos[1]));
	}

}