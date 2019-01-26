#include "eyetracking_interface.h"

#include <stdexcept>

static void log(void* log_context, tobii_log_level_t level, char const* text)
{
	// Synchronize access to the log printout
	std::lock_guard<std::mutex> lock(*(std::mutex*)log_context);

	if (level == TOBII_LOG_LEVEL_ERROR)
		std::cerr << "Logged error: " << text << std::endl;
}

eyetracking_interface::eyetracking_interface() : exit_thread(false)
{
	init_eyetracking();
}

eyetracking_interface::~eyetracking_interface()
{
	// Cleanup subscriptions and resources
	auto error = tobii_gaze_point_unsubscribe(device);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to unsubscribe from gaze stream." << std::endl;

	exit_thread = true;
	thread.join();

	error = tobii_device_destroy(device);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to destroy device." << std::endl;

	error = tobii_api_destroy(api);
	if (error != TOBII_ERROR_NO_ERROR)
		std::cerr << "Failed to destroy API." << std::endl;
}

void eyetracking_interface::init_eyetracking()
{
	// Create log mutex used for thread synchronization in log function
	std::mutex log_mutex;
	tobii_custom_log_t custom_log{ &log_mutex, log };

	auto error = tobii_api_create(&api, nullptr, &custom_log);
	if (error != TOBII_ERROR_NO_ERROR)
	{
		throw std::runtime_error("Failed to initialize the Tobii Stream Engine API.");
	}

	auto devices = list_devices(api);
	if (devices.size() == 0)
	{
		tobii_api_destroy(api);
		throw std::runtime_error("No stream engine compatible device(s) found.");
	}
	// Select the first device, usually there is only one in the research tests
	auto selected_device = devices[0];
	std::cout << "Connecting to " << selected_device << "." << std::endl;

	error = tobii_device_create(api, selected_device.c_str(), &device);
	if (error != TOBII_ERROR_NO_ERROR)
	{
		tobii_api_destroy(api);
		throw std::runtime_error("Failed to initialize the device with url.");

	}

	// Start the background processing thread before subscribing to data
	thread = std::thread(
		[this]()
	{
		while (!exit_thread)
		{
			// Do a timed blocking wait for new gaze data, will time out after some hundred milliseconds
			auto error = tobii_wait_for_callbacks(NULL, 1, &device);

			if (error == TOBII_ERROR_TIMED_OUT) continue; // If timed out, redo the wait for callbacks call

			if (error == TOBII_ERROR_CONNECTION_FAILED)
			{
				// Block here while attempting reconnect, if it fails, exit the thread
				error = reconnect(device);
				if (error != TOBII_ERROR_NO_ERROR)
				{
					std::cerr << "Connection was lost and reconnection failed." << std::endl;
					return;
				}
				continue;
			}
			else if (error != TOBII_ERROR_NO_ERROR)
			{
				std::cerr << "tobii_wait_for_callbacks failed: " << tobii_error_message(error) << "." << std::endl;
				return;
			}
			// Calling this function will execute the subscription callback functions
			error = tobii_device_process_callbacks(device);

			if (error == TOBII_ERROR_CONNECTION_FAILED)
			{
				// Block here while attempting reconnect, if it fails, exit the thread
				error = reconnect(device);
				if (error != TOBII_ERROR_NO_ERROR)
				{
					std::cerr << "Connection was lost and reconnection failed." << std::endl;
					return;
				}
				continue;
			}
			else if (error != TOBII_ERROR_NO_ERROR)
			{
				std::cerr << "tobii_device_process_callbacks failed: " << tobii_error_message(error) << "." << std::endl;
				return;
			}
		}
	});

	// Start subscribing to gaze and supply lambda callback function to handle the gaze point data
	error = tobii_gaze_point_subscribe(device,
		[](tobii_gaze_point_t const* gaze_point, void* user_data)
	{
		(void)user_data; // Unused parameter
		if (gaze_point->validity == TOBII_VALIDITY_VALID) {
			auto tmpData = eyetracking_data();
			tmpData.positionX = gaze_point->position_xy[0];
			tmpData.positionY = gaze_point->position_xy[1];
			tmpData.timestamp_us = gaze_point->timestamp_us;

			auto eyetrackingData = (std::atomic<eyetracking_data>*) user_data;
			eyetrackingData->store(tmpData);

			//std::cout << "Gaze point: " << gaze_point->timestamp_us << " " << gaze_point->position_xy[0]
			//	<< ", " << gaze_point->position_xy[1] << std::endl;
		}
		else
			std::cout << "Gaze point: " << gaze_point->timestamp_us << " INVALID" << std::endl;
	}, &mEyetrackingData);
	if (error != TOBII_ERROR_NO_ERROR)
	{
		exit_thread = true;
		thread.join();
		tobii_device_destroy(device);
		tobii_api_destroy(api);
		throw std::runtime_error("Failed to subscribe to gaze stream.");
	}
}

std::vector<std::string> eyetracking_interface::list_devices(tobii_api_t* api)
{
	std::vector<std::string> result;
	auto error = tobii_enumerate_local_device_urls(api,
		[](char const* url, void* user_data) // Use a lambda for url receiver function
	{
		// Add url string to the supplied result vector
		auto list = (std::vector<std::string>*) user_data;
		list->push_back(url);
	}, &result);
	if (error != TOBII_ERROR_NO_ERROR) {
		throw std::runtime_error("Failed to enumerate eyetracking devices!");
	}

	return result;
}

tobii_error_t eyetracking_interface::reconnect(tobii_device_t* device)
{
	// Try reconnecting for 10 seconds before giving up
	for (int i = 0; i < 40; ++i)
	{
		auto error = tobii_device_reconnect(device);
		if (error != TOBII_ERROR_CONNECTION_FAILED) return error;
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	return TOBII_ERROR_CONNECTION_FAILED;
}

eyetracking_data eyetracking_interface::get_eyetracking_data()
{
	return mEyetrackingData.load();
}