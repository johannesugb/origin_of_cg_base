#pragma once
#include "cg_base.h"

namespace cgb
{
	class window
	{
#if defined(USE_OPENGL46_CONTEXT)
		friend class generic_glfw;
		friend class opengl46;
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

		/** @brief Consecutive ID, identifying a window.
		 *	First window will get the ID=0, second one ID=1, etc.  */
		uint32_t id() const { return mWindowId; }

		/** Returns the user-defined name of this window which
		 *	can be set via @ref set_name */
		const std::string& name() const { return mName; }

		/** Returns the window handle or std::nullopt if
		 *	it wasn't constructed successfully, has been moved from,
		 *	or has been destroyed. */
		std::optional<window_handle> handle() const { return mHandle; }

		/** Returns the inner window size, i.e. the extent of the pixels which can be rendered into */
		glm::uvec2 resolution() const;

		/** Returns the aspect ratio of the window, which is width/height */
		float aspect_ratio() const;

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
		void set_resolution(glm::uvec2 pExtent);

		/** Set a new title */
		void set_title(std::string pTitle);

		/** Change the monitor. This method can be used to switch
		 *  between full-screen and windowed modes.
		 *  TODO: Test and verify that statement ^ 
		 */
		void change_monitor(std::optional<monitor_handle> pMonitor);


	private:
		/** Static variable which holds the ID that the next window will get assigned */
		static uint32_t mNextWindowId;

		/** Unique window id */
		uint32_t mWindowId; 

		/** A name which used to identify this window => make sure, it is unique! */
		std::string mName;

		/** Handle of this window */
		std::optional<window_handle> mHandle;

		/** This window's title */
		std::string mTitle;

		/** Monitor this window is attached to, if set */
		std::optional<monitor_handle> mMonitor;
	};
}