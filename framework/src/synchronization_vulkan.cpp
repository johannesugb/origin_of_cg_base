#include "synchronization_vulkan.h"

namespace cgb
{
	fence fence::create(const vk::FenceCreateInfo& pCreateInfo)
	{
		fence result;
		result.mCreateInfo = pCreateInfo;
		result.mFence = context().logical_device().createFenceUnique(pCreateInfo);
		return result;
	}

	semaphore::~semaphore()
	{
		if (mCustomDeleter) {
			// If there is a custom deleter => call it now
			(*mCustomDeleter)();
			mCustomDeleter.reset();
		}
		// Destroy the dependant semaphore before destroying "my actual semaphore"
		// ^ This is ensured by the order of the members
		//   See: https://isocpp.org/wiki/faq/dtors#calling-member-dtors
	}

	semaphore semaphore::create(const vk::SemaphoreCreateInfo& pCreateInfo)
	{ 
		semaphore result;
		result.mCreateInfo = pCreateInfo;
		result.mSemaphore = context().logical_device().createSemaphoreUnique(pCreateInfo);
		return result;
	}
}
