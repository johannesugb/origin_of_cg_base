#pragma once


struct prev_frame_data {
	glm::mat4 mvpMatrix;
	glm::vec2 imgSize;
};

struct taa_prev_frame_data
{
	glm::mat4 vPMatrix;
	glm::mat4 invPMatrix;
	glm::mat4 invVMatrix;
	glm::vec2 jitter;
};

struct dynamic_image_resource
{
	std::shared_ptr<cgb::vulkan_image> colorImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> resolveColorImages;
};
