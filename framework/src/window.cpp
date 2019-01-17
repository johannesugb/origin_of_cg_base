#include "window.h"

namespace cgb
{
	uint32_t window::mNextWindowId = 0u;

	window::window(window_handle handle) :
		mWindowId(mNextWindowId++),
		mName(),
		mHandle(std::move(handle)),
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
		mTitle(std::move(other.mTitle)),
		mMonitor(std::move(other.mMonitor))
	{
		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;
	}

	window& window::operator =(window&& other) noexcept
	{
		mName = std::move(other.mName);
		mHandle = std::move(other.mHandle);
		mTitle = std::move(other.mTitle);
		mMonitor = std::move(other.mMonitor);

		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;

		return *this;
	}

	void window::set_name(std::string pName)
	{
		mName = pName;
	}

	glm::uvec2 window::resolution() const
	{
		return context().window_extent(*this);
	}

	float window::aspect_ratio() const
	{
		auto res = resolution();
		return static_cast<float>(res.x) / static_cast<float>(res.y);
	}

	void window::set_resolution(glm::uvec2 pExtent)
	{
		context().set_window_size(*this, pExtent);
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

}