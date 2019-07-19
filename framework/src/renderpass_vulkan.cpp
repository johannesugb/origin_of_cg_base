namespace cgb
{
	using namespace boolinq;

	renderpass renderpass::create(std::initializer_list<attachment> pAttachments)
	{
		

		renderpass result;
		for (auto it = std::begin(pAttachments); it != pAttachments.end(); it++) {
			const auto& a = *it;

			// 1. Create the attachment descriptions
			result.mAttachmentDescriptions.push_back(vk::AttachmentDescription()
				.setFormat(a.format().mFormat)
				.setSamples(to_vk_sample_count(a.sample_count()))
				// At this point, we can not really make a guess about load/store ops, so.. don't care:
				.setLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setFinalLayout( // The layout is a bit of an (educated) guess:
					a.is_depth_attachment()
					? vk::ImageLayout::eDepthStencilAttachmentOptimal
					: vk::ImageLayout::eColorAttachmentOptimal));

			auto attachmentIndex = static_cast<uint32_t>(result.mAttachmentDescriptions.size() - 1);
			size_t i = 0;
			
			// 2. Depending on the type, fill one of the references
			if (a.is_color_attachment()) {
				if (a.has_specific_location()) {
					auto gotSomething = insert_at_location_and_possibly_get_already_existing_element(result.mOrderedColorAttachmentRefs,
						a.location(), attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
					if (gotSomething.has_value()) {
						// See if we have a problem
						// TODO: Test if this behaves correctly:
						auto loc = std::find_if(std::begin(pAttachments), it, [&current = a](const attachment& previous) {
							return previous.is_color_attachment() && previous.has_specific_location() && previous.location() == current.location();
						});
						if (loc == it) {
							// Everything's good, we just need to find another place fot the existing attachment
							insert_at_first_unused_location_or_push_back(result.mOrderedColorAttachmentRefs,
								gotSomething->attachment, gotSomething->layout);
						}
					}
				}
				else {
					// No specific location => set at first position which is unused
					insert_at_first_unused_location_or_push_back(result.mOrderedColorAttachmentRefs,
						attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
				}
			}
		}
	}
}
