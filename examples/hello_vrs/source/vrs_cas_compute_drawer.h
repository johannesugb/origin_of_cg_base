#pragma once
#include "vulkan_drawer.h"


class vrs_cas_compute_drawer :
	public cgb::vulkan_drawer
{
public:
	vrs_cas_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages);
	virtual ~vrs_cas_compute_drawer();

	virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects);

	void set_width_height(int width, int height) { mWidth = width; mHeight = height; }

private:
	static const int WORKGROUP_SIZE = 16;

	int mWidth;
	int mHeight;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderBlitImages;

	void blit_image(vk::CommandBuffer& commandBuffer);
};

