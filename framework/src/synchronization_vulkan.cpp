#include "synchronization_vulkan.h"

namespace cgb
{
#pragma region fence
	fence::fence() noexcept
		: mCreateInfo{}
		, mFence{nullptr}
	{ }

	fence::fence(const vk::FenceCreateInfo& pCreateInfo, const vk::Fence& pFence) noexcept
		: mCreateInfo{pCreateInfo}
		, mFence{pFence}
	{ }

	fence::fence(fence&& other) noexcept
		: mCreateInfo{ std::move(other.mCreateInfo) }
		, mFence{ std::move(other.mFence) }
	{ 
		other.mCreateInfo = {};
		other.mFence = nullptr;
	}

	fence& fence::operator=(fence&& other) noexcept
	{ 
		mCreateInfo = std::move(other.mCreateInfo);
		mFence = std::move(other.mFence);
		other.mCreateInfo = {};
		other.mFence = nullptr;
		return *this;
	}

	fence::~fence()
	{ 
		if (mFence) {
			context().logical_device().destroyFence(mFence);
			mFence = nullptr;
		}
	}

	fence fence::create(const vk::FenceCreateInfo& pCreateInfo)
	{ 
		return fence{
			pCreateInfo,
			context().logical_device().createFence(pCreateInfo)
			//
			//
			// FUUU, das dürfte sich wohl ziemlich auszahlen, alles auf UniqueHandle umzustellen GODDAMNIT!!
			//
			//
		};
	}
#pragma endregion

#pragma region semaphore
	semaphore::semaphore() noexcept
		: mCreateInfo{}
		, mSemaphore{nullptr}
		, mCustomDeleter{}
		, mDependantSemaphore{}
	{ }

	semaphore::semaphore(const vk::SemaphoreCreateInfo& pCreateInfo, const vk::Semaphore& pSemaphore) noexcept
		: mCreateInfo{pCreateInfo}
		, mSemaphore{pSemaphore}
		, mCustomDeleter{}
		, mDependantSemaphore{}
	{ }

	semaphore::semaphore(semaphore&& other) noexcept
		: mCreateInfo{ std::move(other.mCreateInfo) }
		, mSemaphore{ std::move(other.mSemaphore) }
		, mCustomDeleter{ std::move(other.mCustomDeleter) }
		, mDependantSemaphore{ std::move(other.mDependantSemaphore) }
	{ 
		other.mCreateInfo = {};
		other.mSemaphore = nullptr;
		other.mCustomDeleter.reset(); // TODO: This is probably not required
		other.mDependantSemaphore.reset(); // TODO: This is probably not required
	}

	semaphore& semaphore::operator=(semaphore&& other) noexcept
	{ 
		mCreateInfo = std::move(other.mCreateInfo);
		mSemaphore = std::move(other.mSemaphore);
		mCustomDeleter = std::move(other.mCustomDeleter);
		mDependantSemaphore = std::move(other.mDependantSemaphore);
		other.mCreateInfo = {};
		other.mSemaphore = nullptr;
		other.mCustomDeleter.reset(); // TODO: This is probably not required
		other.mDependantSemaphore.reset(); // TODO: This is probably not required
		return *this;
	}

	semaphore::~semaphore()
	{
		if (mCustomDeleter) {
			// If there is a custom deleter => call it now
			(*mCustomDeleter)();
			mCustomDeleter.reset();
		}
		if (mDependantSemaphore) {
			// Destroy the dependant semaphore before destroying myself
			mDependantSemaphore.reset();
		}
		if (mSemaphore) {
			context().logical_device().destroySemaphore(mSemaphore);
			mSemaphore = nullptr;
		}
	}

	semaphore semaphore::create(const vk::SemaphoreCreateInfo& pCreateInfo)
	{ 
		return semaphore{
			pCreateInfo,
			context().logical_device().createSemaphore(pCreateInfo)
		};
	}
#pragma endregion
}
