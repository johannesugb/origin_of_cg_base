#pragma once

namespace cgb
{
	/** A synchronization object which allows GPU->CPU synchronization */
	class fence
	{
	public:
		fence() = default;
		fence(const fence&) = delete;
		fence(fence&&) = default;
		fence& operator=(const fence&) = delete;
		fence& operator=(fence&&) = default;
		~fence() = default;

		const auto& create_info() const { return mCreateInfo; }
		const auto& handle() const { return mFence.get(); }
		const auto* handle_addr() const { return &mFence.get(); }

		static fence create(const vk::FenceCreateInfo& pCreateInfo);

	private:
		vk::FenceCreateInfo mCreateInfo;
		vk::UniqueFence mFence;
	};

	/** A synchronization object which allows GPU->GPU synchronization */
	class semaphore
	{
	public:
		semaphore() = default;
		semaphore(const semaphore&) = delete;
		semaphore(semaphore&&) = default;
		semaphore& operator=(const semaphore&) = delete;
		semaphore& operator=(semaphore&&) = default;
		virtual ~semaphore();

		const auto& create_info() const { return mCreateInfo; }
		const auto& handle() const { return mSemaphore.get(); }
		const auto* handle_addr() const { return &mSemaphore.get(); }

		static semaphore create(const vk::SemaphoreCreateInfo& pCreateInfo);

	private:
		vk::SemaphoreCreateInfo mCreateInfo;
		vk::UniqueSemaphore mSemaphore;

		// --- Some advanced features of a semaphore object ---

		/** An optional dependant semaphore. This means: The dependant
		*	semaphore can be assumed to be finished when this semaphore
		*	has finished.
		*	The point here is that some internal function might wait on it,
		*	that shall be somewhat opaque to the user in some cases.
		*	The dependant semaphore child object ensures that the semaphore
		*	does not get destructed prematurely.
		*/
		std::optional<semaphore> mDependantSemaphore;

		/** A custom deleter function called upon destruction of this semaphore */
		std::optional<std::function<void()>> mCustomDeleter;

	};

}
