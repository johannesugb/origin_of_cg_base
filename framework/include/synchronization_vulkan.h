#pragma once

namespace cgb
{
	/** A synchronization object which allows GPU->CPU synchronization */
	struct fence
	{
		fence() noexcept;
		fence(const vk::FenceCreateInfo&, const vk::Fence&) noexcept;
		fence(const fence&) = delete;
		fence(fence&&) noexcept;
		fence& operator=(const fence&) = delete;
		fence& operator=(fence&&) noexcept;
		~fence();

		static fence create(const vk::FenceCreateInfo& pCreateInfo);

		vk::FenceCreateInfo mCreateInfo;
		vk::Fence mFence;
	};

	/** A synchronization object which allows GPU->GPU synchronization */
	struct semaphore
	{
		semaphore() noexcept;
		semaphore(const vk::SemaphoreCreateInfo&, const vk::Semaphore&) noexcept;
		semaphore(const semaphore&) = delete;
		semaphore(semaphore&&) noexcept;
		semaphore& operator=(const semaphore&) = delete;
		semaphore& operator=(semaphore&&) noexcept;
		virtual ~semaphore();

		static semaphore create(const vk::SemaphoreCreateInfo& pCreateInfo);

		vk::SemaphoreCreateInfo mCreateInfo;
		vk::Semaphore mSemaphore;

		// --- Some advanced features of a semaphore object ---

		/** A custom deleter function called upon destruction of this semaphore */
		std::optional<std::function<void()>> mCustomDeleter;

		/** An optional dependant semaphore. This means: The dependant
		*	semaphore can be assumed to be finished when this semaphore
		*	has finished.
		*	The point here is that some internal function might wait on it,
		*	that shall be somewhat opaque to the user in some cases.
		*	The dependant semaphore child object ensures that the semaphore
		*	does not get destructed prematurely.
		*/
		std::optional<semaphore> mDependantSemaphore;
	};

}
