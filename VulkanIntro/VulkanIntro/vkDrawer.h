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
	vkDrawer(std::shared_ptr<vkCommandBufferManager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline);
	virtual ~vkDrawer();

	virtual void draw(std::vector<vkRenderObject*> renderObjects);

	void set_vrs_images(std::vector<std::shared_ptr<vkCgbImage>> vrsImages) { mVrsImages = vrsImages; }
protected:
	std::shared_ptr<vkCommandBufferManager> mCommandBufferManager;
	std::vector<std::shared_ptr<vkCgbImage>> mVrsImages;

	std::shared_ptr<vulkan_pipeline> mPipeline;

	void record_secondary_command_buffer(std::vector<vkRenderObject*> renderObjects);
};

