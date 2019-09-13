#pragma once
#include "vulkan_context.h"
#include "vulkan_image.h"

#include <string>
#include <memory>

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

class save_image {

public:
	save_image();
	~save_image();

	save_image & operator=(const save_image & s);

	void save_image_to_file(std::shared_ptr<cgb::vulkan_image> image, vk::ImageLayout imageLayout, std::string filename);
private:
	std::vector<std::shared_ptr<std::thread>> threadList;
};