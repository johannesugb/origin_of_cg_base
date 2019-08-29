#pragma once
#include "vrs_image_compute_drawer_base.h"

struct vrs_mas_comp_data
{
	glm::mat4 vPMatrix;
	glm::mat4 invPMatrix;
	glm::mat4 invVMatrix;
	glm::vec2 projAScale;
	glm::vec2 imgSize;
};

class vrs_mas_compute_drawer :
	public vrs_image_compute_drawer_base
{
public:
	vrs_mas_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderImages,
		std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages, std::vector<std::shared_ptr<cgb::vulkan_image>> motionVecImages,
		std::vector<std::shared_ptr<cgb::vulkan_image>> motionVecBlitImages);
	virtual ~vrs_mas_compute_drawer();

	virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer);

	virtual void set_cam_data(UniformBufferObject camData, float nearPlane, float farPlane);

	vrs_mas_comp_data get_push_constant_data() { return mCurrPushConstData; }

private:
	std::vector<UniformBufferObject> mCamData;
	std::vector<UniformBufferObject> mPrevCamData;
	float mNearPlane;
	float mFarPlane;

	vrs_mas_comp_data mCurrPushConstData;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderBlitImages;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mMotionVecImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mMotionVecBlitImages;

	void blit_image(vk::CommandBuffer& commandBuffer);
};

