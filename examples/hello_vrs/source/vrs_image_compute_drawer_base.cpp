#include "vrs_image_compute_drawer_base.h"

vrs_image_compute_drawer_base::vrs_image_compute_drawer_base(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDebugImages)
	: vulkan_drawer(commandBufferManager, pipeline, globalResourceBundles), mVrsDebugImages(vrsDebugImages)
{
}

vrs_image_compute_drawer_base::~vrs_image_compute_drawer_base()
{
}
