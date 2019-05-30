#pragma once

#include <vulkan_drawer.h>
#include <vulkan_pipeline.h>
#include <vulkan_render_object.h>
#include <vulkan_renderer.h>
#include <vulkan_resource_bundle.h>
#include <vulkan_resource_bundle_layout.h>
#include <vulkan_resource_bundle_group.h>
#include <vulkan_attribute_description_binding.h>

#include "common_structs.h"

class deferred_renderer
{
public:
	deferred_renderer(std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers, std::shared_ptr<cgb::vulkan_render_queue> vulkanRenderQueue,
		std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager, dynamic_image_resource targetImage, std::vector<dynamic_image_resource> additionalTargetImages,
		vk::Viewport viewport, vk::Rect2D scissor, std::shared_ptr<cgb::vulkan_resource_bundle_layout> objectResourceBundleLayout, std::shared_ptr<cgb::vulkan_resource_bundle> lightsResourceBundle,
		std::shared_ptr<cgb::vulkan_attribute_description_binding> materialAttr, std::shared_ptr<cgb::vulkan_resource_bundle_group> resourceBundleGroup, 
		std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages, cgb::quake_camera& camera);
	virtual ~deferred_renderer();

	void init_vulkan(std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers, std::shared_ptr<cgb::vulkan_render_queue> vulkanRenderQueue,
		std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager, dynamic_image_resource targetImage, std::vector<dynamic_image_resource> additionalTargetImages,
		vk::Viewport viewport, vk::Rect2D scissor, std::shared_ptr<cgb::vulkan_resource_bundle_layout> objectResourceBundleLayout, std::shared_ptr<cgb::vulkan_resource_bundle> lightsResourceBundle,
		std::shared_ptr<cgb::vulkan_attribute_description_binding> materialAttr, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages);

	void draw(std::vector<cgb::vulkan_render_object*> renderObjects);

	std::shared_ptr<cgb::vulkan_renderer> get_final_renderer() { return mLightingPassRenderer; }

	void allocate_resources();

	void reload_shaders();

private:
	std::shared_ptr<cgb::vulkan_resource_bundle_group> mResourceBundleGroup; 
	cgb::quake_camera* mCamera;

	std::unique_ptr<cgb::vulkan_drawer> mGeoPassDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mGeoPassPipeline;
	std::shared_ptr<cgb::vulkan_framebuffer> mGeoPassFramebuffer;
	std::shared_ptr<cgb::vulkan_renderer> mGeoPassRenderer;

	std::unique_ptr<cgb::vulkan_drawer> mLightingPassDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mLightingPassPipeline;
	std::shared_ptr<cgb::vulkan_framebuffer> mLightingPassFramebuffer;
	std::shared_ptr<cgb::vulkan_renderer> mLightingPassRenderer;

	std::unique_ptr<cgb::vulkan_render_object> mLightingPassFullScreenQuad;

	// framebuffer attachments
	std::shared_ptr<cgb::vulkan_image> mNormalsMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mNormalsImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mNormalsTextures;

	std::shared_ptr<cgb::vulkan_image> mAmbientMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mAmbientImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mAmbientTextures;

	std::shared_ptr<cgb::vulkan_image> mDiffuseMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mDiffuseImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mDiffuseTextures;

	std::shared_ptr<cgb::vulkan_image> mSpecularMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mSpecularImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mSpecularTextures;

	std::vector<std::shared_ptr<cgb::vulkan_texture>> mDepthTextures;

	std::shared_ptr<cgb::vulkan_resource_bundle> mGeoBufferResourceBundle;

	void draw_geo_pass(std::vector<cgb::vulkan_render_object*> renderObjects);

	void draw_lighting_pass();

	void create_framebuffer_attachments(dynamic_image_resource targetImage);
	void create_framebuffer_msaa_attachment(std::shared_ptr<cgb::vulkan_image>& msaaImage,
		std::vector<std::shared_ptr<cgb::vulkan_image>>& images, std::vector<std::shared_ptr<cgb::vulkan_texture>>& textures,
		uint32_t width, uint32_t height, vk::Format colorFormat);
};

