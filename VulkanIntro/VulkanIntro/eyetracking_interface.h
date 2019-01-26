#pragma once

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>


struct eyetracking_data {
	float positionX;
	float positionY;
	int64_t timestamp_us;
};

class eyetracking_interface
{
public:
	eyetracking_interface();
	~eyetracking_interface();

	eyetracking_data get_eyetracking_data();
private:
	tobii_api_t* api;
	tobii_device_t* device;		
	
	std::thread thread;
	// used to stop the eyetracking callback thread
	std::atomic<bool> exit_thread;
	std::atomic<eyetracking_data> mEyetrackingData;


	void init_eyetracking();

	std::vector<std::string> list_devices(tobii_api_t* api);
	tobii_error_t reconnect(tobii_device_t* device);
};

