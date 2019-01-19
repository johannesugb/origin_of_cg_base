#pragma once

#include <vector>

#include "vkContext.h"

#include "vkCommandBufferManager.h"
#include "vkRenderObject.h"

#include "vulkan_pipeline.h"

class vkDrawer
{
public:
	vkDrawer(vkCommandBufferManager* commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline);
	virtual ~vkDrawer();

	void draw(std::vector<vkRenderObject*> renderObjects);
private:
	vkCommandBufferManager * mCommandBufferManager;

	std::shared_ptr<vulkan_pipeline> mPipeline;

	void record_secondary_command_buffer(std::vector<vkRenderObject*> renderObjects);
};

