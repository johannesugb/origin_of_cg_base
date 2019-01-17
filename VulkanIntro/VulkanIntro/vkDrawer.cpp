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
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);

		vk::Buffer vertexBuffers[] = { renderObject->get_vertex_buffer() , renderObject->get_vertex_buffer() };
		vk::DeviceSize offsets[] = { 0, 0 };
		commandBuffer.bindVertexBuffers(1, 2, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(renderObject->get_index_buffer(), 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, 1, &(renderObject->get_descriptor_set()), 0, nullptr);

		//renderObject->updateUniformBuffer(frameIndex, 0, swapChainExtent);

		commandBuffer.pushConstants(
			mPipelineLayout,
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(PushUniforms),
			&(renderObject->get_push_uniforms()));

		commandBuffer.drawIndexed(static_cast<uint32_t>(renderObject->get_indices().size()), 1, 0, 0, 0);
	}
}