#pragma once
#include "vulkan_drawer.h"

struct vrs_cas_comp_data
{
	glm::mat4 vPMatrix;
	glm::mat4 invPMatrix;
	glm::mat4 invVMatrix;
	glm::vec2 projAScale;
	glm::vec2 imgSize;
} ;

class vrs_cas_compute_drawer :
	public cgb::vulkan_drawer
{
public:
	vrs_cas_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages);
	virtual ~vrs_cas_compute_drawer();

	virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects);

	void set_width_height(int width, int height) { mWidth = width; mHeight = height; }

	void set_cam_data(UniformBufferObject camData, float nearPlane, float farPlane);

private:
	static const int WORKGROUP_SIZE = 16;

	int mWidth;
	int mHeight;

	UniformBufferObject mCamData;
	UniformBufferObject mPrevCamData;
	float mNearPlane;
	float mFarPlane;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderBlitImages;

	void blit_image(vk::CommandBuffer& commandBuffer);
};

