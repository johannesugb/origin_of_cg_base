#pragma once
#include "vrs_image_compute_drawer_base.h"

#include "eyetracking_interface.h"


class vrs_eye_tracked_blit :
	public vrs_image_compute_drawer_base
{
public:
	vrs_eye_tracked_blit(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDebugImages);
	virtual ~vrs_eye_tracked_blit();

	virtual void draw(std::vector<cgb::vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer);

	void set_descriptor_sets(std::vector<vk::DescriptorSet> descriptorSets) { mDescriptorSets = descriptorSets; }
	void set_eye_inf(std::shared_ptr<eyetracking_interface> eyeInf) { mEyeInf = eyeInf; }

	void precompute();

private:
	std::vector<vk::DescriptorSet> mDescriptorSets;
	std::shared_ptr<eyetracking_interface> mEyeInf;

	vk::DescriptorSetLayout vrsComputeDebugDescriptorSetLayout;
	vk::DescriptorPool vrsComputeDebugDescriptorPool;
	std::vector<vk::DescriptorSet> mVrsComputeDebugDescriptorSets;
		
	std::unique_ptr<cgb::vulkan_image> mPrecomputedImage;

	void createVrsComputeDescriptorSetLayout();
	void createVrsComputeDescriptorPool();
	void createVrsDescriptorSets();


	void draw_initial(vk::CommandBuffer& commandBuffer);
	void blit_initial_image(vk::CommandBuffer& commandBuffer);
};