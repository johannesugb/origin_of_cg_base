#include "vkDrawer.h"

#include "vulkan_framebuffer.h"


vkDrawer::vkDrawer(std::shared_ptr<vkCommandBufferManager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline) : mCommandBufferManager(commandBufferManager), mPipeline(pipeline)
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
	inheritanceInfo.renderPass = vkContext::instance().vulkanFramebuffer->get_render_pass();
	inheritanceInfo.framebuffer = vkContext::instance().vulkanFramebuffer->get_swapchain_framebuffer();
	inheritanceInfo.subpass = 0;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue| vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);
	// bind pipeline for this draw command
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->get_pipeline());	

	if (vkContext::instance().shadingRateImageSupported) {
		commandBuffer.bindShadingRateImageNV(mVrsImages[vkContext::instance().currentFrame]->get_image_view(), vk::ImageLayout::eShadingRateOptimalNV, vkContext::instance().dynamicDispatchInstanceDevice);
	}

	for (vkRenderObject* renderObject : renderObjects) {

		vk::Buffer vertexBuffers[] = { renderObject->get_vertex_buffer() , renderObject->get_vertex_buffer() };
		vk::DeviceSize offsets[] = { 0, 0 };
		commandBuffer.bindVertexBuffers(1, 2, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(renderObject->get_index_buffer(), 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipeline->get_pipeline_layout(), 0, 1, &(renderObject->get_descriptor_set()), 0, nullptr);

		commandBuffer.pushConstants(
			mPipeline->get_pipeline_layout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(PushUniforms),
			&(renderObject->get_push_uniforms()));

		commandBuffer.drawIndexed(static_cast<uint32_t>(renderObject->get_indices().size()), 1, 0, 0, 0);
	}
}