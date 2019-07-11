namespace cgb
{
	pipeline pipeline::create(depth_test pDepthTest, depth_write pDepthWrite)
	{
		pipeline result;
		result.mDepthStencilConfig
			.setDepthTestEnable(to_vk_bool(pDepthTest.mEnabled))
			.setDepthWriteEnable(to_vk_bool(pDepthWrite.mEnabled))
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(VK_FALSE)
			.setFront(vk::StencilOpState())
			.setBack(vk::StencilOpState());
	}
}
