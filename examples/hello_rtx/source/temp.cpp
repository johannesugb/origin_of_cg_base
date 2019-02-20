#include "temp.h"

// ============================================================
// Resource base
// ============================================================

VkPhysicalDevice ResourceBase::_physicalDevice;
VkDevice ResourceBase::_device;
VkPhysicalDeviceMemoryProperties ResourceBase::_physicalDeviceMemoryProperties;
VkCommandPool ResourceBase::_commandPool;
VkQueue ResourceBase::_transferQueue;

void ResourceBase::Init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue transferQueue)
{
	_physicalDevice = physicalDevice;
	_device = device;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
	_commandPool = commandPool;
	_transferQueue = transferQueue;
}

uint32_t ResourceBase::GetMemoryType(VkMemoryRequirements& memoryRequiriments, VkMemoryPropertyFlags memoryProperties)
{
	uint32_t result = 0;
	for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < VK_MAX_MEMORY_TYPES; ++memoryTypeIndex) {
		if (memoryRequiriments.memoryTypeBits & (1 << memoryTypeIndex)) {
			if ((_physicalDeviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & memoryProperties) == memoryProperties) {
				result = memoryTypeIndex;
				break;
			}
		}
	}
	return result;
}



// ============================================================
// Buffer resource
// ============================================================

BufferResource::~BufferResource()
{
	Cleanup();
}

VkResult BufferResource::Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties)
{
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	Size = size;

	VkResult code = vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &Buffer);
	if (code != VK_SUCCESS) {
		Buffer = VK_NULL_HANDLE;
		return code;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_device, Buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryType(memoryRequirements, memoryProperties);

	code = vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &Memory);
	if (code != VK_SUCCESS) {
		vkDestroyBuffer(_device, Buffer, nullptr);
		Buffer = VK_NULL_HANDLE;
		Memory = VK_NULL_HANDLE;
		return code;
	}

	code = vkBindBufferMemory(_device, Buffer, Memory, 0);
	if (code != VK_SUCCESS) {
		vkDestroyBuffer(_device, Buffer, nullptr);
		vkFreeMemory(_device, Memory, nullptr);
		Buffer = VK_NULL_HANDLE;
		Memory = VK_NULL_HANDLE;
		return code;
	}

	return code;
}

void BufferResource::Cleanup()
{
	if (Buffer) {
		vkDestroyBuffer(_device, Buffer, nullptr);
		Buffer = VK_NULL_HANDLE;
	}
	if (Memory) {
		vkFreeMemory(_device, Memory, nullptr);
		Memory = VK_NULL_HANDLE;
	}
}

void* BufferResource::Map(VkDeviceSize size) const
{
	void* mappedMemory = nullptr;
	const VkResult code = vkMapMemory(_device, Memory, 0, size, 0, &mappedMemory);
	if (code != VK_SUCCESS) {
		throw std::runtime_error("vkMapMemory");
		return nullptr;
	}
	return mappedMemory;
}

void BufferResource::Unmap() const
{
	vkUnmapMemory(_device, Memory);
}

bool BufferResource::CopyToBufferUsingMapUnmap(const void* memoryToCopyFrom, VkDeviceSize size) const
{
	void* mappedMemory = Map(size);
	if (mappedMemory == nullptr) {
		return false;
	}

	memcpy(mappedMemory, memoryToCopyFrom, size);
	Unmap();
	return true;
}