#pragma once
#include "vulkan_drawer.h"

class vrs_image_compute_drawer_base :
	public cgb::vulkan_drawer
{
public: 
	vrs_image_compute_drawer_base(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles = {}, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDebugImages = {});
	virtual ~vrs_image_compute_drawer_base();

	//virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer) = 0;
	
	void set_width_height(int width, int height) { mWidth = width; mHeight = height; }
	virtual void set_cam_data(UniformBufferObject camData, float nearPlane, float farPlane) {};

protected:
	static const int WORKGROUP_SIZE = 16;

	std::vector <std::shared_ptr<cgb::vulkan_image>> mVrsDebugImages;

	int mWidth;
	int mHeight;
};

