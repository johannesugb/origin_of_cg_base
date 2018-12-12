#pragma once

#include <vector>

#include "vkContext.h"

#include "vkCommandBufferManager.h"
#include "vkRenderObject.h"

class vkDrawer
{
public:
	vkDrawer(vkCommandBufferManager* commandBufferManager, VkPipeline &graphicsPipeline,
		VkPipelineLayout &pipelineLayout);
	virtual ~vkDrawer();

	void draw(std::vector<vkRenderObject*> renderObjects);
private:
	vkCommandBufferManager * _commandBufferManager;

	VkPipeline _graphicsPipeline;
	VkPipelineLayout _pipelineLayout;

	void recordSecondaryCommandBuffer(std::vector<vkRenderObject*> renderObjects);
};

