namespace cgb
{
	descriptor_pool::descriptor_pool() noexcept
		: mDescriptorPool()
	{ }

	descriptor_pool::descriptor_pool(const vk::DescriptorPool& pDescriptorPool)
		: mDescriptorPool(pDescriptorPool)
	{ }

	descriptor_pool::descriptor_pool(descriptor_pool&& other) noexcept
		: mDescriptorPool(std::move(other.mDescriptorPool))
	{
		other.mDescriptorPool = nullptr;
	}

	descriptor_pool& descriptor_pool::operator=(descriptor_pool&& other) noexcept
	{
		mDescriptorPool = std::move(other.mDescriptorPool);
		other.mDescriptorPool = nullptr;
		return *this;
	}

	descriptor_pool::~descriptor_pool()
	{
		if (mDescriptorPool) {
			context().logical_device().destroyDescriptorPool(mDescriptorPool);
			mDescriptorPool = nullptr;
		}
	}

	descriptor_set::descriptor_set() noexcept 
		: mDescriptorSet() 
	{ }

	descriptor_set::descriptor_set(const vk::DescriptorSet& pDescriptorSet)
		: mDescriptorSet(pDescriptorSet)
	{ }

	descriptor_set::descriptor_set(descriptor_set&& other) noexcept
		: mDescriptorSet(std::move(other.mDescriptorSet))
	{ 
		other.mDescriptorSet = nullptr;
	}

	descriptor_set& descriptor_set::operator=(descriptor_set&& other) noexcept
	{ 
		mDescriptorSet = std::move(other.mDescriptorSet);
		other.mDescriptorSet = nullptr;
		return *this;
	}
}
