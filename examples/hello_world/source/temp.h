#pragma once


#define NVVK_CHECK_ERROR(code, message) \
    { \
        if (code != VK_SUCCESS) \
        { \
            throw std::runtime_error(message); \
        } \
    }


class ResourceBase
{
protected:
	static VkPhysicalDevice _physicalDevice;
	static VkDevice _device;
	static VkPhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;
	static VkCommandPool _commandPool;
	static VkQueue _transferQueue;

public:
	static void Init(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue transferQueue);
	static uint32_t GetMemoryType(VkMemoryRequirements& memoryRequiriments, VkMemoryPropertyFlags memoryProperties);
};

class BufferResource : public ResourceBase
{
public:
	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	VkDeviceSize Size = 0;
	
public:
	~BufferResource();

public:
	VkResult Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
	void Cleanup();

	void* Map(VkDeviceSize size) const;
	void Unmap() const;

	bool CopyToBufferUsingMapUnmap(const void* memoryToCopyFrom, VkDeviceSize size) const;
};