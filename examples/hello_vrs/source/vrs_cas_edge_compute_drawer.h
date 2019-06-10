#pragma once
#include "vrs_cas_compute_drawer.h"

class vrs_cas_edge_compute_drawer :
	public vrs_cas_compute_drawer
{
public:
	vrs_cas_edge_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsEdgeImages,
		std::vector<std::shared_ptr<cgb::vulkan_image>> vrsEdgeBlitImages);
	virtual ~vrs_cas_edge_compute_drawer();

private:
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsEdgeImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsEdgeBlitImages;

	virtual void blit_image(vk::CommandBuffer& commandBuffer);
};

