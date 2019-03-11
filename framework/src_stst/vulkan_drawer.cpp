#include "vulkan_drawer.h"

#include "vulkan_framebuffer.h"

namespace cgb {
	vulkan_drawer::vulkan_drawer(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline) : mCommandBufferManager(commandBufferManager), mPipeline(pipeline)
	{
	}


	vulkan_drawer::~vulkan_drawer()
	{
	}

	void vulkan_drawer::draw(std::vector<vulkan_render_object*> renderObjects)
	{
		record_secondary_command_buffer(renderObjects);
	}

	void vulkan_drawer::record_secondary_command_buffer(std::vector<vulkan_render_object*> renderObjects) {
		vk::CommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.renderPass = vulkan_context::instance().vulkanFramebuffer->get_render_pass();
		inheritanceInfo.framebuffer = vulkan_context::instance().vulkanFramebuffer->get_swapchain_framebuffer();
		inheritanceInfo.subpass = 0;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);
		// bind pipeline for this draw command
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->get_pipeline());

		if (vulkan_context::instance().shadingRateImageSupported) {
			commandBuffer.bindShadingRateImageNV(mVrsImages[vulkan_context::instance().currentFrame]->get_image_view(), vk::ImageLayout::eShadingRateOptimalNV, vulkan_context::instance().dynamicDispatchInstanceDevice);
		}

		for (vulkan_render_object* renderObject : renderObjects) {

			vk::Buffer vertexBuffers[] = { renderObject->get_vertex_buffer(0) , renderObject->get_vertex_buffer(0) };
			vk::DeviceSize offsets[] = { 0, 0 };
			commandBuffer.bindVertexBuffers(1, 2, vertexBuffers, offsets);
			commandBuffer.bindIndexBuffer(renderObject->get_index_buffer(), 0, vk::IndexType::eUint32);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipeline->get_pipeline_layout(), 0, 1, &(renderObject->get_resource_bundle()->get_descriptor_set()), 0, nullptr);

			commandBuffer.pushConstants(
				mPipeline->get_pipeline_layout(),
				vk::ShaderStageFlagBits::eVertex,
				0,
				sizeof(PushUniforms),
				&(renderObject->get_push_uniforms()));

			commandBuffer.drawIndexed(static_cast<uint32_t>(renderObject->get_index_count()), 1, 0, 0, 0);
		}
	}
}