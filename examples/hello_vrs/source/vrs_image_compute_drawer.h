#pragma once
#include "vulkan_drawer.h"

#include "eyetracking_interface.h"

struct vrs_eye_comp_data {
	glm::vec2 eyePoint;
	glm::vec2 imgSize;
};

namespace cgb {

	class vrs_image_compute_drawer :
		public vulkan_drawer
	{
	public:
		vrs_image_compute_drawer(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline,
			std::vector<std::shared_ptr<vulkan_image>> vrsDebugImages);
		virtual ~vrs_image_compute_drawer();

		virtual void draw(std::vector<vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer);

		void set_descriptor_sets(std::vector<vk::DescriptorSet> descriptorSets) { mDescriptorSets = descriptorSets; }
		void set_width_height(int width, int height) { mWidth = width; mHeight = height; }
		void set_eye_inf(std::shared_ptr<eyetracking_interface> eyeInf) { mEyeInf = eyeInf; }

	private:
		std::vector<vk::DescriptorSet> mDescriptorSets;
		std::vector <std::shared_ptr<vulkan_image>> mVrsDebugImages;
		std::shared_ptr<eyetracking_interface> mEyeInf;

		int mWidth;
		int mHeight;

		static const int WORKGROUP_SIZE = 16;

		vk::DescriptorSetLayout vrsComputeDebugDescriptorSetLayout;
		vk::DescriptorPool vrsComputeDebugDescriptorPool;
		std::vector<vk::DescriptorSet> mVrsComputeDebugDescriptorSets;

		void createVrsComputeDescriptorSetLayout();
		void createVrsComputeDescriptorPool();
		void createVrsDescriptorSets();
	};

}