#include "cg_stdafx.h"
#include "vulkan_attribute_description_binding.h"


vulkan_attribute_description_binding::vulkan_attribute_description_binding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate)
{
	mBindingDescription = {};
	mBindingDescription.binding = 1;
	mBindingDescription.stride = stride;
	mBindingDescription.inputRate = vk::VertexInputRate::eVertex;
}


vulkan_attribute_description_binding::~vulkan_attribute_description_binding()
{
}

void vulkan_attribute_description_binding::add_attribute_description(uint32_t location, vk::Format format, uint32_t offset)
{
	auto attributeDescription = vk::VertexInputAttributeDescription();
	attributeDescription.binding = mBindingDescription.binding;
	attributeDescription.location = location;
	attributeDescription.format = format;
	attributeDescription.offset = offset;
	mAttributeDescriptions.push_back(attributeDescription);
}