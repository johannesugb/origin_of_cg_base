#pragma once

#include <vector>

#include "vkContext.h"

#include "vkCommandBufferManager.h"
#include "vkRenderObject.h"

#include "vulkan_pipeline.h"
#include "vkCgbImage.h"

class vkDrawer
{
public:
	vkDrawer(vkCommandBufferManager* commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline);
	virtual ~vkDrawer();

	virtual void draw(std::vector<vkRenderObject*> renderObjects);

	void set_vrs_image(std::shared_ptr<vkCgbImage> vrsImage) { mVrsImage = vrsImage; }
private:
	vkCommandBufferManager * mCommandBufferManager;
	std::shared_ptr<vkCgbImage> mVrsImage;

	std::shared_ptr<vulkan_pipeline> mPipeline;

	void record_secondary_command_buffer(std::vector<vkRenderObject*> renderObjects);
};

