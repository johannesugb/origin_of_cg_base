#pragma once

#include <vector>

#include "vkContext.h"

#include "vkCommandBufferManager.h"
#include "vkRenderObject.h"

class vkDrawer
{
public:
	vkDrawer(vkCommandBufferManager* commandBufferManager, vk::Pipeline &graphicsPipeline,
		vk::PipelineLayout &pipelineLayout);
	virtual ~vkDrawer();

	void draw(std::vector<vkRenderObject*> renderObjects);
private:
	vkCommandBufferManager * mCommandBufferManager;

	vk::Pipeline mGraphicsPipeline;
	vk::PipelineLayout mPipelineLayout;

	void record_secondary_command_buffer(std::vector<vkRenderObject*> renderObjects);
};

