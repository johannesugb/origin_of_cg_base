#include "deferred_renderer.h"



deferred_renderer::deferred_renderer(std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers, std::shared_ptr<cgb::vulkan_render_queue> vulkanRenderQueue,
	std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager, dynamic_image_resource targetImage, std::vector<dynamic_image_resource> additionalTargetImages,
	vk::Viewport viewport, vk::Rect2D scissor, std::shared_ptr<cgb::vulkan_resource_bundle_layout> objectResourceBundleLayout, std::shared_ptr<cgb::vulkan_resource_bundle> lightsResourceBundle,
	std::shared_ptr<cgb::vulkan_attribute_description_binding> materialAttr, std::shared_ptr<cgb::vulkan_resource_bundle_group> resourceBundleGroup, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages,
	cgb::quake_camera& camera) :
	mResourceBundleGroup(resourceBundleGroup), mCamera(&camera)
{
	init_vulkan(dependentRenderers, vulkanRenderQueue, drawCommandBufferManager, targetImage, additionalTargetImages, viewport, scissor, objectResourceBundleLayout, lightsResourceBundle,
		materialAttr, vrsImages);
}


deferred_renderer::~deferred_renderer()
{
}

void deferred_renderer::init_vulkan(std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers, std::shared_ptr<cgb::vulkan_render_queue> vulkanRenderQueue,
	std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager, dynamic_image_resource targetImage, std::vector<dynamic_image_resource> additionalTargetImages,
	vk::Viewport viewport, vk::Rect2D scissor, std::shared_ptr<cgb::vulkan_resource_bundle_layout> objectResourceBundleLayout, std::shared_ptr<cgb::vulkan_resource_bundle> lightsResourceBundle,
	std::shared_ptr<cgb::vulkan_attribute_description_binding> materialAttr, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages) {

	create_framebuffer_attachments(targetImage);

	auto geoBufferResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
	geoBufferResourceBundleLayout->add_binding(0, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
	geoBufferResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
	geoBufferResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
	geoBufferResourceBundleLayout->add_binding(3, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
	geoBufferResourceBundleLayout->add_binding(4, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
	geoBufferResourceBundleLayout->bake();
	mGeoBufferResourceBundle = mResourceBundleGroup->create_resource_bundle(geoBufferResourceBundleLayout, true);
	mGeoBufferResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eShaderReadOnlyOptimal, mNormalsTextures);
	mGeoBufferResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, mAmbientTextures);
	mGeoBufferResourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, mSpecularTextures);
	mGeoBufferResourceBundle->add_dynamic_image_resource(3, vk::ImageLayout::eShaderReadOnlyOptimal, mDiffuseTextures);
	mGeoBufferResourceBundle->add_dynamic_image_resource(4, vk::ImageLayout::eShaderReadOnlyOptimal, mDepthTextures);

	mLightingPassFullScreenQuad = std::make_unique<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> {});

	mGeoPassRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, vulkanRenderQueue, drawCommandBufferManager, dependentRenderers);
	mLightingPassRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, vulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>> { mGeoPassRenderer });

	mGeoPassFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(targetImage.colorImage->get_width(), targetImage.colorImage->get_height(),
		cgb::vulkan_context::instance().dynamicRessourceCount, cgb::vulkan_context::instance().msaaSamples);
	mGeoPassFramebuffer->add_dynamic_color_attachment(mNormalsMsaaImage, mNormalsImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	mGeoPassFramebuffer->add_dynamic_color_attachment(mAmbientMsaaImage, mAmbientImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	mGeoPassFramebuffer->add_dynamic_color_attachment(mSpecularMsaaImage, mSpecularImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	mGeoPassFramebuffer->add_dynamic_color_attachment(mDiffuseMsaaImage, mDiffuseImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	mGeoPassFramebuffer->set_depth_attachment(cgb::vulkan_context::instance().defaultDepthImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eUndefined, vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore);
	for (auto addTargetImg : additionalTargetImages) {
		mGeoPassFramebuffer->add_dynamic_color_attachment(addTargetImg.colorImage, addTargetImg.resolveColorImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	}
	mGeoPassFramebuffer->bake();
	mGeoPassRenderer->set_framebuffer(mGeoPassFramebuffer);

	mLightingPassFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(targetImage.colorImage->get_width(), targetImage.colorImage->get_height(),
		cgb::vulkan_context::instance().dynamicRessourceCount, cgb::vulkan_context::instance().msaaSamples);
	mLightingPassFramebuffer->add_dynamic_color_attachment(targetImage.colorImage, targetImage.resolveColorImages, vk::ImageLayout::eShaderReadOnlyOptimal);
	mLightingPassFramebuffer->bake();
	mLightingPassRenderer->set_framebuffer(mLightingPassFramebuffer);

	mGeoPassPipeline = std::make_shared<cgb::vulkan_pipeline>(mGeoPassFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { cgb::MaterialData::get_resource_bundle_layout(), objectResourceBundleLayout }, sizeof(prev_frame_data), cgb::ShaderStageFlagBits::eVertex | cgb::ShaderStageFlagBits::eFragment);
	mGeoPassPipeline->add_attr_desc_binding(materialAttr);
	mGeoPassPipeline->add_color_blend_attachment_state(mGeoPassPipeline->get_color_blend_attachment_state(0));
	mGeoPassPipeline->add_color_blend_attachment_state(mGeoPassPipeline->get_color_blend_attachment_state(0));
	mGeoPassPipeline->add_color_blend_attachment_state(mGeoPassPipeline->get_color_blend_attachment_state(0));
	mGeoPassPipeline->add_color_blend_attachment_state(mGeoPassPipeline->get_color_blend_attachment_state(0));
	mGeoPassPipeline->add_color_blend_attachment_state(mGeoPassPipeline->get_color_blend_attachment_state(0));
	mGeoPassPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/deferred/blinnphong_nm_geo.vert.spv");
	mGeoPassPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/deferred/blinnphong_nm_geo.frag.spv");
	mGeoPassPipeline->bake();
	mGeoPassDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mGeoPassPipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { });
	if (cgb::vulkan_context::instance().shadingRateImageSupported) {
		mGeoPassDrawer->set_vrs_images(vrsImages);
	}

	auto posAndUv = std::make_shared<cgb::vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
	posAndUv->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
	posAndUv->add_attribute_description(1, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

	mLightingPassPipeline = std::make_shared<cgb::vulkan_pipeline>(mLightingPassFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { lightsResourceBundle->get_resource_bundle_layout(), geoBufferResourceBundleLayout }, sizeof(taa_prev_frame_data), cgb::ShaderStageFlagBits::eVertex | cgb::ShaderStageFlagBits::eFragment);
	mLightingPassPipeline->add_attr_desc_binding(posAndUv);
	mLightingPassPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/deferred/lighting_pass.vert.spv");
	if (cgb::vulkan_context::instance().msaaSamples == vk::SampleCountFlagBits::e1) {
		mLightingPassPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/deferred/lighting_pass.frag.spv");
	}
	else {
		mLightingPassPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/deferred/lighting_pass_msaa.frag.spv");
	}
	mLightingPassPipeline->bake();
	mLightingPassDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mLightingPassPipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { lightsResourceBundle, mGeoBufferResourceBundle });
	if (cgb::vulkan_context::instance().shadingRateImageSupported) {
		mLightingPassDrawer->set_vrs_images(vrsImages);
	}
}

void deferred_renderer::draw(std::vector<cgb::vulkan_render_object*> renderObjects) {
	draw_geo_pass(renderObjects);
	draw_lighting_pass();
}

void deferred_renderer::draw_geo_pass(std::vector<cgb::vulkan_render_object*> renderObjects) {
	mGeoPassRenderer->render({ renderObjects }, mGeoPassDrawer.get());
}

void deferred_renderer::draw_lighting_pass() {
	auto pushConst = std::make_shared<taa_prev_frame_data>();
	pushConst->invPMatrix = glm::inverse(mCamera->projection_matrix());
	mLightingPassFullScreenQuad->update_push_constant(pushConst);
	mLightingPassRenderer->render({ mLightingPassFullScreenQuad.get() }, mLightingPassDrawer.get());
}

void deferred_renderer::allocate_resources()
{
	mResourceBundleGroup->allocate_resource_bundle(mGeoBufferResourceBundle.get());
}

void deferred_renderer::reload_shaders()
{
	mGeoPassPipeline->bake();
	mLightingPassPipeline->bake();
}

void deferred_renderer::create_framebuffer_attachments(dynamic_image_resource targetImage) {
	vk::Format normalFormat = vk::Format::eR32G32B32A32Sfloat;
	vk::Format ambientFormat = vk::Format::eR8G8B8A8Unorm;
	vk::Format diffuseFormat = vk::Format::eR8G8B8A8Unorm;
	vk::Format specularFormat = vk::Format::eR8G8B8A8Unorm;

	create_framebuffer_msaa_attachment(mNormalsMsaaImage, mNormalsImages, mNormalsTextures, targetImage.colorImage->get_width(), targetImage.colorImage->get_height(), normalFormat);
	create_framebuffer_msaa_attachment(mAmbientMsaaImage, mAmbientImages, mAmbientTextures, targetImage.colorImage->get_width(), targetImage.colorImage->get_height(), ambientFormat);
	create_framebuffer_msaa_attachment(mDiffuseMsaaImage, mDiffuseImages, mDiffuseTextures, targetImage.colorImage->get_width(), targetImage.colorImage->get_height(), diffuseFormat);
	create_framebuffer_msaa_attachment(mSpecularMsaaImage, mSpecularImages, mSpecularTextures, targetImage.colorImage->get_width(), targetImage.colorImage->get_height(), specularFormat);

	mDepthTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
	for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
		mDepthTextures[i] = std::make_shared<cgb::vulkan_texture>(cgb::vulkan_context::instance().defaultDepthImage);
	}
}

void deferred_renderer::create_framebuffer_msaa_attachment(std::shared_ptr<cgb::vulkan_image>& msaaImage,
	std::vector<std::shared_ptr<cgb::vulkan_image>>& images, std::vector<std::shared_ptr<cgb::vulkan_texture>>& textures,
	uint32_t width, uint32_t height, vk::Format colorFormat) {
	msaaImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, cgb::vulkan_context::instance().msaaSamples, colorFormat,
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
	images.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
	textures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

	for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
		images[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, vk::SampleCountFlagBits::e1, colorFormat,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		images[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
		textures[i] = std::make_shared<cgb::vulkan_texture>(images[i]);
	}

}
