#include "window_vulkan.h"

namespace cgb
{
	window::window()
		: window_base()
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
		: window_base(std::move(other))
		, mSurfaceFormatSelector(std::move(other.mSurfaceFormatSelector))
		, mPresentationModeSelector(std::move(other.mPresentationModeSelector))
		, mNumberOfSamplesGetter(std::move(other.mNumberOfSamplesGetter))
		, mMultisampleCreateInfoBuilder(std::move(other.mMultisampleCreateInfoBuilder))
	{
	}

	window& window::operator= (window&& other) noexcept
	{
		window_base::operator=(std::move(other));
		mSurfaceFormatSelector = std::move(other.mSurfaceFormatSelector);
		mPresentationModeSelector = std::move(other.mPresentationModeSelector);
		mNumberOfSamplesGetter = std::move(other.mNumberOfSamplesGetter);
		mMultisampleCreateInfoBuilder = std::move(other.mMultisampleCreateInfoBuilder);
		return *this;
	}

	void window::request_srgb_framebuffer(bool pRequestSrgb)
	{
		// Which formats are supported, depends on the surface.
		mSurfaceFormatSelector = [srgbFormatRequested = pRequestSrgb](const vk::SurfaceKHR & pSurface) {
			// Get all the formats which are supported by the surface:
			auto srfFrmts = context().physical_device().getSurfaceFormatsKHR(pSurface);

			// Init with a default format...
			auto selSurfaceFormat = vk::SurfaceFormatKHR{
				vk::Format::eB8G8R8A8Unorm,
				vk::ColorSpaceKHR::eSrgbNonlinear
			};

			// ...and try to possibly find one which is definitely supported or better suited w.r.t. the surface.
			if (!(srfFrmts.size() == 1 && srfFrmts[0].format == vk::Format::eUndefined)) {
				for (const auto& e : srfFrmts) {
					if (srgbFormatRequested) {
						if (is_srgb_format(cgb::image_format(e))) {
							selSurfaceFormat = e;
							break;
						}
					}
					else {
						if (!is_srgb_format(cgb::image_format(e))) {
							selSurfaceFormat = e;
							break;
						}
					}
				}
			}

			// In any case, return a format
			return selSurfaceFormat;
		};

		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_presentaton_mode(cgb::presentation_mode pMode)
	{
		mPresentationModeSelector = [presMode = pMode](const vk::SurfaceKHR & pSurface) {
			// Supported presentation modes must be queried from a device:
			auto presModes = context().physical_device().getSurfacePresentModesKHR(pSurface);

			// Select a presentation mode:
			decltype(presModes)::iterator selPresModeItr = presModes.end();
			switch (presMode) {
			case cgb::presentation_mode::immediate:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eImmediate);
				break;
			case cgb::presentation_mode::double_buffering:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eFifoRelaxed);
				break;
			case cgb::presentation_mode::vsync:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eFifo);
				break;
			case cgb::presentation_mode::triple_buffering:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eMailbox);
				break;
			default:
				throw std::runtime_error("should not get here");
			}
			if (selPresModeItr == presModes.end()) {
				LOG_WARNING_EM("No presentation mode specified or desired presentation mode not available => will select any presentation mode");
				selPresModeItr = presModes.begin();
			}

			return *selPresModeItr;
		};

		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_number_of_samples(int pNumSamples)
	{
		vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
		switch (pNumSamples) {
		case 1:
			samples = vk::SampleCountFlagBits::e1;
			break;
		case 2:
			samples = vk::SampleCountFlagBits::e2;
			break;
		case 4:
			samples = vk::SampleCountFlagBits::e4;
			break;
		case 8:
			samples = vk::SampleCountFlagBits::e8;
			break;
		case 16:
			samples = vk::SampleCountFlagBits::e16;
			break;
		case 32:
			samples = vk::SampleCountFlagBits::e32;
			break;
		case 64:
			samples = vk::SampleCountFlagBits::e64;
			break;
		default:
			throw std::invalid_argument("Invalid number of samples");
		}

		mNumberOfSamplesGetter = [samples]() { return samples; };

		mMultisampleCreateInfoBuilder = [this]() {
			auto samples = mNumberOfSamplesGetter();
			return vk::PipelineMultisampleStateCreateInfo()
				.setSampleShadingEnable(vk::SampleCountFlagBits::e1 == samples ? VK_FALSE : VK_TRUE) // disable/enable?
				.setRasterizationSamples(samples)
				.setMinSampleShading(1.0f) // Optional
				.setPSampleMask(nullptr) // Optional
				.setAlphaToCoverageEnable(VK_FALSE) // Optional
				.setAlphaToOneEnable(VK_FALSE); // Optional
		};

		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_number_of_presentable_images(uint32_t pNumImages)
	{
		mNumberOfPresentableImagesGetter = [numImages = pNumImages]() { return numImages; };

		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::set_number_of_concurrent_frames(uint32_t pNumConcurrent)
	{
		mNumberOfConcurrentFramesGetter = [numConcurrent = pNumConcurrent]() { return numConcurrent; };

		// If the window has already been created, the new setting can't 
		// be applied unless the window is being recreated.
		if (is_alive()) {
			mRecreationRequired = true;
		}
	}

	void window::open()
	{
		context().dispatch_to_main_thread([this]() {
			while (context().work_off_event_handlers() > 0u);

			// Share the graphics context between all windows
			auto* sharedContex = context().get_window_for_shared_context();
			// Bring window into existance:
			auto* handle = glfwCreateWindow(mRequestedSize.mWidth, mRequestedSize.mHeight,
				mTitle.c_str(),
				mMonitor.has_value() ? mMonitor->mHandle : nullptr,
				sharedContex);
			if (nullptr == handle) {
				// No point in continuing
				throw new std::runtime_error("Failed to create window with the title '" + mTitle + "'");
			}
			mHandle = window_handle{ handle };

			// There will be some pending work regarding this newly created window stored within the
			// context's events, like creating a swap chain and so on. 
			// Why wait? Invoke them now!
			while (context().work_off_event_handlers() > 0u);
		});
	}

	vk::SurfaceFormatKHR window::get_config_surface_format(const vk::SurfaceKHR & surface)
	{
		if (!mSurfaceFormatSelector) {
			// Set the default:
			request_srgb_framebuffer(false);
		}
		// Determine the format:
		return mSurfaceFormatSelector(surface);
	}

	vk::PresentModeKHR window::get_config_presentation_mode(const vk::SurfaceKHR & surface)
	{
		if (!mPresentationModeSelector) {
			// Set the default:
			set_presentaton_mode(cgb::presentation_mode::triple_buffering);
		}
		// Determine the presentation mode:
		return mPresentationModeSelector(surface);
	}

	vk::SampleCountFlagBits window::get_config_number_of_samples()
	{
		if (!mNumberOfSamplesGetter) {
			// Set the default:
			set_number_of_samples(1);
		}
		// Determine the number of samples:
		return mNumberOfSamplesGetter();
	}

	vk::PipelineMultisampleStateCreateInfo window::get_config_multisample_state_create_info()
	{
		if (!mMultisampleCreateInfoBuilder) {
			// Set the default:
			set_number_of_samples(1);
		}
		// Get the config struct:
		return mMultisampleCreateInfoBuilder();
	}

	uint32_t window::get_config_number_of_presentable_images()
	{
		if (!mNumberOfPresentableImagesGetter) {
			auto srfCaps = context().physical_device().getSurfaceCapabilitiesKHR(surface());
			auto imageCount = srfCaps.minImageCount + 1u;
			if (srfCaps.maxImageCount > 0) { // A value of 0 for maxImageCount means that there is no limit
				imageCount = glm::min(imageCount, srfCaps.maxImageCount);
			}
			return imageCount;
		}
		return mNumberOfPresentableImagesGetter();
	}

	uint32_t window::get_config_number_of_concurrent_frames()
	{
		if (!mNumberOfConcurrentFramesGetter) {
			return get_config_number_of_presentable_images();
		}
		return mNumberOfConcurrentFramesGetter();
	}

	void window::set_extra_semaphore_dependency_for_frame(semaphore pSemaphore, uint64_t pFrameId)
	{
		if (pFrameId < mCurrentFrame)
	}

	std::vector<semaphore> window::remove_all_extra_semaphore_dependencies_for_frame(uint64_t pFrameId)
	{

	}

	std::vector<semaphore> window::set_num_extra_semaphores_to_generate_per_frame(uint32_t pNumExtraSemaphores)
	{

	}

}