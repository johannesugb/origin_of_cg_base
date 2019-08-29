#pragma once
#include "vrs_image_compute_drawer_base.h"

struct vrs_cas_comp_data
{
	glm::mat4 vPMatrix;
	glm::mat4 invPMatrix;
	glm::mat4 invVMatrix;
	glm::vec2 projAScale;
	glm::vec2 imgSize;
} ;

class vrs_cas_compute_drawer :
	public vrs_image_compute_drawer_base
{
public:
	vrs_cas_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages);
	virtual ~vrs_cas_compute_drawer();

	virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer);

	void set_width_height(int width, int height) { mWidth = width; mHeight = height; }

	virtual void set_cam_data(UniformBufferObject camData, float nearPlane, float farPlane);

	vrs_cas_comp_data get_push_constant_data() { return mCurrPushConstData; }

protected:
	std::vector<UniformBufferObject> mCamData;
	std::vector<UniformBufferObject> mPrevCamData;
	float mNearPlane;
	float mFarPlane;

	vrs_cas_comp_data mCurrPushConstData;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderBlitImages;

	virtual void blit_image(vk::CommandBuffer& commandBuffer);
};

