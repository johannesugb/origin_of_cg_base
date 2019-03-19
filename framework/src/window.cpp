#include "window.h"

namespace cgb
{
	uint32_t window::mNextWindowId = 0u;

	window::window(window_handle handle) 
		: mIsInUse(false)
		, mWindowId(mNextWindowId++)
		, mName()
		, mHandle(std::move(handle))
		, mTitle()
		, mMonitor()
		, mIsInputEnabled(true)
	{

	}

	window::~window()
	{
		if (mHandle) {
			context().close_window(*this);
			mHandle = std::nullopt;
		}
	}

	window::window(window&& other) noexcept
		: mIsInUse(std::move(other.mIsInUse))
		, mWindowId(std::move(other.mWindowId))
		, mName(std::move(other.mName))
		, mHandle(std::move(other.mHandle))
		, mTitle(std::move(other.mTitle))
		, mMonitor(std::move(other.mMonitor))
		, mIsInputEnabled(std::move(other.mIsInputEnabled))
	{
		other.mIsInUse = false;
		other.mWindowId = 0u;
		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;
		other.mIsInputEnabled = false;
	}

	window& window::operator =(window&& other) noexcept
	{
		mIsInUse = std::move(other.mIsInUse);
		mWindowId = std::move(other.mWindowId);
		mName = std::move(other.mName);
		mHandle = std::move(other.mHandle);
		mTitle = std::move(other.mTitle);
		mMonitor = std::move(other.mMonitor);
		mIsInputEnabled = std::move(other.mIsInputEnabled);

		other.mIsInUse = false;
		other.mWindowId = 0u;
		other.mName = "moved from";
		other.mHandle = std::nullopt;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;
		other.mIsInputEnabled = false;

		return *this;
	}

	void window::set_is_in_use(bool value)
	{
		mIsInUse = value;
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
		if (mHandle.has_value()) {
			glfwSetWindowTitle(mHandle->mHandle, pTitle.c_str());
		}
	}

	void window::change_monitor(std::optional<monitor_handle> pMonitor)
	{
		mMonitor = pMonitor;
		// TODO: assign the window to the monitor
	}

	void window::set_is_input_enabled(bool pValue)
	{
		mIsInputEnabled = pValue; 
	}

}