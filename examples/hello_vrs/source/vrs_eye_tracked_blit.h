#pragma once
#include "vulkan_drawer.h"

#include "eyetracking_interface.h"

namespace cgb {

	class vrs_eye_tracked_blit :
		public vulkan_drawer
	{
	public:
		vrs_eye_tracked_blit(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline,
			std::vector<std::shared_ptr<vulkan_image>> vrsDebugImages);
		virtual ~vrs_eye_tracked_blit();

		virtual void draw(std::vector<vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer);

		void set_descriptor_sets(std::vector<vk::DescriptorSet> descriptorSets) { mDescriptorSets = descriptorSets; }
		void set_width_height(int width, int height) { mWidth = width; mHeight = height; }
		void set_eye_inf(std::shared_ptr<eyetracking_interface> eyeInf) { mEyeInf = eyeInf; }

		void precompute();

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
		
		std::unique_ptr<vulkan_image> mPrecomputedImage;

		void createVrsComputeDescriptorSetLayout();
		void createVrsComputeDescriptorPool();
		void createVrsDescriptorSets();


		void draw_initial(vk::CommandBuffer& commandBuffer);
		void blit_initial_image(vk::CommandBuffer& commandBuffer);
	};

}