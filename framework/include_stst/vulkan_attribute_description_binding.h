#pragma once

#include "vulkan_context.h"

class vulkan_attribute_description_binding
{
public:
	vulkan_attribute_description_binding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate);
	~vulkan_attribute_description_binding();

	void add_attribute_description(uint32_t location = 0, vk::Format format = vk::Format::eUndefined, uint32_t offset = 0);

	vk::VertexInputBindingDescription get_binding_description() {
		return mBindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() {
		return mAttributeDescriptions;
	}

private:
	vk::VertexInputBindingDescription mBindingDescription;
	std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;
};

