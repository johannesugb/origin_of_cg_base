#pragma once

#include <vector>

#include "vulkan_context.h"

#include "vulkan_command_buffer_manager.h"
#include "vulkan_render_object.h"

#include "vulkan_pipeline.h"
#include "vulkan_image.h"

namespace cgb {

	class vulkan_drawer
	{
	public:
		vulkan_drawer(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline);
		virtual ~vulkan_drawer();

		virtual void draw(std::vector<vulkan_render_object*> renderObjects);

		void set_vrs_images(std::vector<std::shared_ptr<vulkan_image>> vrsImages) { mVrsImages = vrsImages; }
	protected:
		std::shared_ptr<vulkan_command_buffer_manager> mCommandBufferManager;
		std::vector<std::shared_ptr<vulkan_image>> mVrsImages;

		std::shared_ptr<vulkan_pipeline> mPipeline;

		void record_secondary_command_buffer(std::vector<vulkan_render_object*> renderObjects);
	};

}