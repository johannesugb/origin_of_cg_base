#include "vkDrawer.h"



vkDrawer::vkDrawer(vkCommandBufferManager* commandBufferManager, vk::Pipeline &graphicsPipeline,
	vk::PipelineLayout &pipelineLayout) : mCommandBufferManager(commandBufferManager), mGraphicsPipeline(graphicsPipeline),
	mPipelineLayout(pipelineLayout)
{
}


vkDrawer::~vkDrawer()
{
}

void vkDrawer::draw(std::vector<vkRenderObject*> renderObjects)
{
	record_secondary_command_buffer(renderObjects);
}

void vkDrawer::record_secondary_command_buffer(std::vector<vkRenderObject*> renderObjects) {
	vk::CommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.renderPass = vkContext::instance().renderPass;
	inheritanceInfo.framebuffer = vkContext::instance().frameBuffer;
	inheritanceInfo.subpass = 0;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue| vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);

	for (vkRenderObject* renderObject : renderObjects) {
		// bind pipeline for this draw command
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

		vk::Buffer vertexBuffers[] = { renderObject->getVertexBuffer() , renderObject->getVertexBuffer() };
		vk::DeviceSize offsets[] = { 0, 0 };
		commandBuffer.bindVertexBuffers(1, 2, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(renderObject->getIndexBuffer(), 0, vk::IndexType::eUint32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &(renderObject->getDescriptorSet()), 0, nullptr);

		//renderObject->updateUniformBuffer(frameIndex, 0, swapChainExtent);

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushUniforms),
			&(renderObject->getPushUniforms()));

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(renderObject->getIndices().size()), 1, 0, 0, 0);
	}
}