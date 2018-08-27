#pragma once
#include "cg_base.h"

namespace cgb
{
	class window
	{
#if defined(USE_OPENGL46_CONTEXT)
		friend class generic_glfw;
		friend class opengl46;
#elif defined(USE_OPENGLES32_CONTEXT)
#elif defined(USE_VULKAN_CONTEXT)
		friend class generic_glfw;
		friend class vulkan;
#endif
	public:
		/** Constructs a window with an already existing handle */
		window(window_handle);
		/** Destructor */
		~window();
		// Prevent copying a window:
		window(const window&) = delete;
		/** Move a window */
		window(window&&) noexcept;
		// Prevent copy-assigning a window:
		window& operator =(const window&) = delete;
		/** Move-assign a window */
		window& operator =(window&&) noexcept;

		/** Returns the user-defined name of this window which
		 *	can be set via \ref set_name */
		const std::string& name() const { return mName; }

		/** Returns the window handle or std::nullopt if
		 *	it wasn't constructed successfully, has been moved from,
		 *	or has been destroyed. */
		std::optional<window_handle> handle() const { return mHandle; }

		/** Returns the inner width, i.e. which is usable for rendering */
		int width() const { return mWidth; }

		/** Returns the inner height, i.e. which is usable for rendering */
		int height() const { return mHeight; }

		/** The window title */
		const std::string& title() const { return mTitle; }

		/** Returns the monitor handle or std::nullopt if there is 
		 *  no monitor assigned to this window (e.g. not running
		 *  in full-screen mode.
		 */
		std::optional<monitor_handle> monitor() const { return mMonitor; }

		/** Set a new identifier-name for this window */
		void set_name(std::string pName);

		/** Set a new resolution for this window. This will also update
		 *  this window's underlying framebuffer 
		 *  TODO: Resize underlying framebuffer!
		 */
		void set_resolution(int pWidth, int pHeight);

		/** Set a new title */
		void set_title(std::string pTitle);

		/** Change the monitor. This method can be used to switch
		 *  between full-screen and windowed modes.
		 *  TODO: Test and verify that statement ^ 
		 */
		void change_monitor(std::optional<monitor_handle> pMonitor);

	private:
		/** A name which used to identify this window => make sure, it is unique! */
		std::string mName;

		/** Handle of this window */
		std::optional<window_handle> mHandle;

		/** This window's width */
		int mWidth;

		/** This window's height */
		int mHeight;

		/** This window's title */
		std::string mTitle;

		/** Monitor this window is attached to, if set */
		std::optional<monitor_handle> mMonitor;
	};
}