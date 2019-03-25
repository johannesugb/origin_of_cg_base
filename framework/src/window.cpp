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

	float window::aspect_ratio() const
	{
		auto res = resolution();
		return static_cast<float>(res.x) / static_cast<float>(res.y);
	}

	void window::set_resolution(window_size pExtent)
	{
		if (is_alive()) {
			context().dispatch_to_main_thread([this, pExtent]() {
				glfwSetWindowSize(handle()->mHandle, pExtent.mWidth, pExtent.mHeight);
			});
		}
		else {
			mRequestedSize = pExtent;
		}
	}

	void window::set_title(std::string pTitle)
	{
		mTitle = pTitle;
		if (mHandle.has_value()) {
			context().dispatch_to_main_thread([this, pTitle]() {
				glfwSetWindowTitle(mHandle->mHandle, pTitle.c_str());
			});
		}
	}

	void window::set_is_input_enabled(bool pValue)
	{
		mIsInputEnabled = pValue; 
	}

	void window::switch_to_fullscreen_mode(monitor_handle pOnWhichMonitor)
	{
		if (is_alive()) {
			context().dispatch_to_main_thread([this, pOnWhichMonitor]() {
				glfwSetWindowMonitor(handle()->mHandle, pOnWhichMonitor.mHandle,
									 0, 0,
									 mRequestedSize.mWidth, mRequestedSize.mHeight, // TODO: Support different resolutions or query the current resolution
									 GLFW_DONT_CARE); // TODO: Support different refresh rates
			});
		}
		else {
			mMonitor = std::nullopt;
		}
	}

	void window::switch_to_windowed_mode()
	{
		if (is_alive()) {
			context().dispatch_to_main_thread([this]() {
				int xpos = 10, ypos = 10;
				glfwGetWindowPos(handle()->mHandle, &xpos, &ypos);
				glfwSetWindowMonitor(handle()->mHandle, nullptr,
									 xpos, ypos,
									 mRequestedSize.mWidth, mRequestedSize.mHeight, // TODO: Support different resolutions or query the current resolution
									 GLFW_DONT_CARE); // TODO: Support different refresh rates
			});
		}
		else {
			mMonitor = std::nullopt;
		}
	}

	glm::dvec2 window::cursor_position() const
	{
		assert(context().are_we_on_the_main_thread());
		//std::lock_guard<std::mutex> lock(sInputMutex);
		glm::dvec2 cursorPos;
		assert(handle() != std::nullopt);
		glfwGetCursorPos(handle()->mHandle, &cursorPos[0], &cursorPos[1]);
		return cursorPos;
	}

	glm::uvec2 window::resolution() const
	{
		assert(context().are_we_on_the_main_thread());
		assert(handle());
		int width, height;
		glfwGetWindowSize(handle()->mHandle, &width, &height);
		return glm::uvec2(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	}

	void window::hide_cursor(bool pHide)
	{
		assert(handle());
		context().dispatch_to_main_thread([this, pHide]() {
			glfwSetInputMode(handle()->mHandle, GLFW_CURSOR,
							 pHide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
		});
	}

	bool window::is_cursor_hidden() const
	{
		assert(context().are_we_on_the_main_thread());
		return glfwGetInputMode(handle()->mHandle, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
	}

	void window::set_cursor_pos(glm::dvec2 pCursorPos)
	{
		assert(handle());
		context().dispatch_to_main_thread([this, pCursorPos]() {
			assert(handle());
			glfwSetCursorPos(handle()->mHandle, pCursorPos.x, pCursorPos.y);
		});
	}

}