#include "vkDrawer.h"



vkDrawer::vkDrawer(vkCommandBufferManager* commandBufferManager, VkPipeline &graphicsPipeline,
	VkPipelineLayout &pipelineLayout) : _commandBufferManager(commandBufferManager), _graphicsPipeline(graphicsPipeline),
	_pipelineLayout(pipelineLayout)
{
}


vkDrawer::~vkDrawer()
{
}

void vkDrawer::draw(std::vector<vkRenderObject*> renderObjects)
{
	recordSecondaryCommandBuffer(renderObjects);
}

void vkDrawer::recordSecondaryCommandBuffer(std::vector<vkRenderObject*> renderObjects) {
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = vkContext::instance().renderPass;
	inheritanceInfo.framebuffer = vkContext::instance().frameBuffer;
	inheritanceInfo.subpass = 0;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	VkCommandBuffer commandBuffer = _commandBufferManager->get_command_buffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, beginInfo);

	for (vkRenderObject* renderObject : renderObjects) {
		// bind pipeline for this draw command
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		VkBuffer vertexBuffers[] = { renderObject->getVertexBuffer() , renderObject->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0, 0 };
		vkCmdBindVertexBuffers(commandBuffer, 1, 2, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, renderObject->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &(renderObject->getDescriptorSet()), 0, nullptr);

		//renderObject->updateUniformBuffer(frameIndex, 0, swapChainExtent);

		vkCmdPushConstants(
			commandBuffer,
			_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushUniforms),
			&(renderObject->getPushUniforms()));

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(renderObject->getIndices().size()), 1, 0, 0, 0);
	}
}