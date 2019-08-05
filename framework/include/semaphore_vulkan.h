#pragma once

namespace cgb
{
	// Forward declaration:
	class device_queue;

	/** A synchronization object which allows GPU->GPU synchronization */
	class semaphore_t
	{
	public:
		semaphore_t() = default;
		semaphore_t(const semaphore_t&) = delete;
		semaphore_t(semaphore_t&&) = default;
		semaphore_t& operator=(const semaphore_t&) = delete;
		semaphore_t& operator=(semaphore_t&&) = default;
		virtual ~semaphore_t();

		/**	Set a queue where this semaphore is designated to be submitted to.
		 *	This is only used for keeping the information of the queue. 
		 *	There is no impact on any internal functionality whether or not a designated queue has been set.
		 */
		semaphore_t& set_designated_queue(device_queue& _Queue);

		template <typename F>
		semaphore_t& set_custom_deleter(F&& _Deleter) 
		{
			mCustomDeleter = std::forward<F>(_Deleter);
			return *this;
		}

		const auto& create_info() const { return mCreateInfo; }
		const auto& handle() const { return mSemaphore.get(); }
		const auto* handle_addr() const { return &mSemaphore.get(); }
		auto has_designated_queue() const { return nullptr != mQueue; }
		auto* designated_queue() const { return mQueue; }

		static cgb::owning_resource<semaphore_t> create(context_specific_function<void(semaphore_t&)> _AlterConfigBeforeCreation = {});

	private:
		vk::SemaphoreCreateInfo mCreateInfo;
		vk::UniqueSemaphore mSemaphore;
		device_queue* mQueue;

		// --- Some advanced features of a semaphore object ---

		/** An optional dependant semaphore. This means: The dependant
		*	semaphore can be assumed to be finished when this semaphore
		*	has finished.
		*	The point here is that some internal function might wait on it,
		*	that shall be somewhat opaque to the user in some cases.
		*	The dependant semaphore child object ensures that the semaphore
		*	does not get destructed prematurely.
		*/
		//std::optional<semaphore> mDependantSemaphore;

		/** A custom deleter function called upon destruction of this semaphore */
		std::optional<std::function<void()>> mCustomDeleter;
	};

	// Typedef for a variable representing an owner of a semaphore
	using semaphore = owning_resource<semaphore_t>;
}