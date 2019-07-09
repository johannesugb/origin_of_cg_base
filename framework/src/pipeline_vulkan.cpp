namespace cgb
{
	pipeline::pipeline() noexcept
		: mRenderPass{}
		, mPipelineLayout{}
		, mPipeline{}
	{ }

	pipeline::pipeline(const vk::PipelineLayout& pPipelineLayout, const vk::Pipeline& pPipeline, vk::RenderPass pRenderPass) noexcept
		: mRenderPass{ pRenderPass }
		, mPipelineLayout{ pPipelineLayout }
		, mPipeline{ pPipeline }
	{ }

	pipeline::pipeline(pipeline&& other) noexcept
		: mRenderPass{ std::move(other.mRenderPass) }
		, mPipelineLayout{ std::move(other.mPipelineLayout) }
		, mPipeline{ std::move(other.mPipeline) }
	{ 
		other.mRenderPass = nullptr;
		other.mPipelineLayout = nullptr;
		other.mPipeline = nullptr;
	}

	pipeline& pipeline::operator=(pipeline&& other) noexcept 
	{ 
		mRenderPass = std::move(other.mRenderPass);
		mPipelineLayout = std::move(other.mPipelineLayout);
		mPipeline = std::move(other.mPipeline);

		other.mRenderPass = nullptr;
		other.mPipelineLayout = nullptr;
		other.mPipeline = nullptr;
		return *this;
	}

	pipeline::~pipeline()
	{
		if (mRenderPass) {
			context().mLogicalDevice.destroyRenderPass(mRenderPass);
			mRenderPass = nullptr;
		}
		if (mPipelineLayout) {
			context().mLogicalDevice.destroyPipelineLayout(mPipelineLayout);
			mPipelineLayout = nullptr;
		}
		if (mPipeline) {
			context().mLogicalDevice.destroyPipeline(mPipeline);
			mPipeline = nullptr;
		}
	}
}
