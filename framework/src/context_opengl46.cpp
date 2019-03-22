#include "context_opengl46.h"

namespace cgb
{
	opengl46::opengl46() 
		: generic_glfw()
		, mInitializationComplete(false)
	{
	}

	void window::request_srgb_framebuffer(bool pRequestSrgb)
	{
		switch (pRequestSrgb) {
		case true:
			mPreCreateActions.push_back(
				[](cgb::window& w) {
					glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
				});
			break;
		default:
			mPreCreateActions.push_back(
				[](cgb::window& w) {
					glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_FALSE);
				});
			break;
		}
		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_presentaton_mode(cgb::presentation_mode pMode)
	{
		switch (pMode) {
		case cgb::presentation_mode::immediate:
			mPreCreateActions.push_back(
				[](cgb::window& w) {
					glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
				});
			mPostCreateActions.push_back(
				[](cgb::window& w) {
					glfwSwapInterval(0);
				});
			break;
		case cgb::presentation_mode::vsync:
			mPreCreateActions.push_back(
				[](cgb::window& w) {
					glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
				});
			mPostCreateActions.push_back(
				[](cgb::window& w) {
					glfwSwapInterval(1);
				});
			break;
		default:
			mPreCreateActions.push_back(
				[](cgb::window& w) {
					glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
				});
			mPostCreateActions.push_back(
				[](cgb::window& w) {
					glfwSwapInterval(0);
				});
			break;
		}
		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_number_of_samples(int pNumSamples)
	{
		mPreCreateActions.push_back(
			[samples = pNumSamples](cgb::window & w) {
				glfwWindowHint(GLFW_SAMPLES, samples);
			});
		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	window* opengl46::create_window(const std::string& pTitle)
	{
		auto* wnd = generic_glfw::prepare_window();
		wnd->mPreCreateActions.push_back([](cgb::window& w) {
			// Set several configuration parameters before actually creating the window:
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

			// Set a default value for the following settings:
			glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_FALSE);
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		});
	
		wnd->mPostCreateActions.push_back([wnd]() {
			if (!context().initialization_completed()) {
				// If context has been newly created in the current call to create_window, 
				// 1) make the newly created context current and
				// 2) use the extension loader to get the proc-addresses (which needs an active context)
				glfwMakeContextCurrent(wnd->handle()->mHandle);
				gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
				glfwFocusWindow(wnd->handle()->mHandle);
				context().mInitializationComplete = true;
			}
		});

		return wnd;
	}

	void window::open()
	{
		for (const auto& fu : mPreCreateActions) {
			fu(*this);
		}

		auto* sharedContex = context().get_window_for_shared_context();
		glfwCreateWindow(mRequestedSize.mWidth, mRequestedSize.mHeight,
						 mTitle.c_str(),
						 mMonitor.has_value() ? mMonitor->mHandle : nullptr,
						 sharedContex);

		for (const auto& action : mPostCreateActions) {
			action(*this);
		}
	}

	bool opengl46::check_error(const char* file, int line)
	{
		bool hasError = false;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			LOG_ERROR(fmt::format("glError int[{:d}] hex[0x{:x}] in file[{}], line[{}]", err, err, file, line));
			hasError = true;
		}
		return hasError;
	}

	void opengl46::finish_pending_work()
	{

	}

	void opengl46::begin_composition()
	{

	}

	void opengl46::end_composition()
	{

	}

	void opengl46::begin_frame()
	{

	}

	void opengl46::end_frame()
	{

	}


}
