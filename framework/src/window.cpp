#include "window.h"

namespace cgb
{
	uint32_t window::mNextWindowId = 0u;

	window::window() 
		: mIsInUse(false)
		, mWindowId(mNextWindowId++)
		, mTitle()
		, mMonitor()
		, mIsInputEnabled(true)
		, mRequestedSize{ 512, 512 }
		, mPreCreateActions()
		, mPostCreateActions()
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
		, mHandle(std::move(other.mHandle))
		, mTitle(std::move(other.mTitle))
		, mMonitor(std::move(other.mMonitor))
		, mIsInputEnabled(std::move(other.mIsInputEnabled))
		, mRequestedSize(std::move(other.mRequestedSize))
		, mPreCreateActions(std::move(other.mPreCreateActions))
		, mPostCreateActions(std::move(other.mPostCreateActions))
	{
		other.mIsInUse = false;
		other.mWindowId = 0u;
		other.mHandle = std::nullopt;
		other.mTitle = "moved from";
		other.mMonitor = std::nullopt;
		other.mIsInputEnabled = false;
	}

	window& window::operator =(window&& other) noexcept
	{
		mIsInUse = std::move(other.mIsInUse);
		mWindowId = std::move(other.mWindowId);
		mHandle = std::move(other.mHandle);
		mTitle = std::move(other.mTitle);
		mMonitor = std::move(other.mMonitor);
		mIsInputEnabled = std::move(other.mIsInputEnabled);
		mRequestedSize = std::move(other.mRequestedSize);
		mPreCreateActions = std::move(other.mPreCreateActions);
		mPostCreateActions = std::move(other.mPostCreateActions);

		other.mIsInUse = false;
		other.mWindowId = 0u;
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

	void window::set_fullscreen(monitor_handle pOnWhichMonitor)
	{
		if (is_alive()) {
			glfwsetmonit
		}
		mMonitor = pOnWhichMonitor;
	}
}