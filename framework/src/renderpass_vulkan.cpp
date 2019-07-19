namespace cgb
{
	using namespace cpplinq;

	renderpass renderpass::create(std::initializer_list<attachment> pAttachments)
	{
		renderpass result;

		std::map<uint32_t, vk::AttachmentReference> mSpecificColorLocations;
		std::queue<vk::AttachmentReference> mArbitraryColorLocations;
		std::map<uint32_t, vk::AttachmentReference> mSpecificDepthLocations;
		std::queue<vk::AttachmentReference> mArbitraryDepthLocations;
		std::map<uint32_t, vk::AttachmentReference> mSpecificResolveLocations;
		std::queue<vk::AttachmentReference> mArbitraryResolveLocations;
		std::map<uint32_t, vk::AttachmentReference> mSpecificInputLocations;
		std::queue<vk::AttachmentReference> mArbitraryInputLocations;

		uint32_t maxColorLoc = 0u;
		uint32_t maxDepthLoc = 0u;
		uint32_t maxResolveLoc = 0u;
		uint32_t maxInputLoc = 0u;
		uint32_t maxPreserveLoc = 0u;

		for (const auto& a : pAttachments) {
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
					: (a.is_to_be_presented() // => not depth => assume color, but what about presenting?
						? vk::ImageLayout::ePresentSrcKHR
						: vk::ImageLayout::eColorAttachmentOptimal))
			);

			auto attachmentIndex = static_cast<uint32_t>(result.mAttachmentDescriptions.size() - 1);

			// 2. Depending on the type, fill one or multiple of the references
			if (a.is_color_attachment()) {
				// Build the Reference
				auto attachmentRef = vk::AttachmentReference().setAttachment(attachmentIndex).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
				// But where to insert it?
				if (a.has_specific_location()) {
					assert(mSpecificColorLocations.count(a.location()) == 0); // assert that it is not already contained
					mSpecificColorLocations[a.location()] = attachmentRef;
					maxColorLoc = std::max(maxColorLoc, a.location());
				}
				else {
					mArbitraryColorLocations.push(attachmentRef);
				}
			}
			if (a.is_depth_attachment()) {
				// Build the Reference
				auto attachmentRef = vk::AttachmentReference().setAttachment(attachmentIndex).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				// But where to insert it?
				if (a.has_specific_location()) {
					assert(mSpecificDepthLocations.count(a.location()) == 0); // assert that it is not already contained
					mSpecificDepthLocations[a.location()] = attachmentRef;
					maxDepthLoc = std::max(maxDepthLoc, a.location());
				}
				else {
					mArbitraryDepthLocations.push(attachmentRef);
				}
			}
			if (a.is_to_be_resolved()) {
				// Build the Reference
				auto attachmentRef = vk::AttachmentReference().setAttachment(attachmentIndex).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
				// But where to insert it?
				if (a.has_specific_location()) {
					assert(mSpecificResolveLocations.count(a.location()) == 0); // assert that it is not already contained
					mSpecificResolveLocations[a.location()] = attachmentRef;
					maxResolveLoc = std::max(maxResolveLoc, a.location());
				}
				else {
					mArbitraryResolveLocations.push(attachmentRef);
				}
			}
			if (a.is_shader_input_attachment()) {
				// Build the Reference
				auto attachmentRef = vk::AttachmentReference().setAttachment(attachmentIndex).setLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // TODO: This layout?
				// But where to insert it?
				if (a.has_specific_location()) {
					assert(mSpecificInputLocations.count(a.location()) == 0); // assert that it is not already contained
					mSpecificInputLocations[a.location()] = attachmentRef;
					maxInputLoc = std::max(maxInputLoc, a.location());
				}
				else {
					mArbitraryInputLocations.push(attachmentRef);
				}
			}
		}

		// 3. Fill all the vectors in the right order:
		const auto unusedAttachmentRef = vk::AttachmentReference().setAttachment(VK_ATTACHMENT_UNUSED);
		//    colors => mOrderedColorAttachmentRefs
		for (uint32_t loc = 0; loc < maxColorLoc || mArbitraryColorLocations.size() > 0; ++loc) {
			if (mSpecificColorLocations.count(loc) > 0) {
				result.mOrderedColorAttachmentRefs.push_back(mSpecificColorLocations[loc]);
			}
			else {
				if (mArbitraryColorLocations.size() > 0) {
					result.mOrderedColorAttachmentRefs.push_back(mArbitraryColorLocations.front());
					mArbitraryColorLocations.pop();
				}
				else {
					result.mOrderedColorAttachmentRefs.push_back(unusedAttachmentRef);
				}
			}
		}
		//    depths => mOrderedDepthAttachmentRefs
		for (uint32_t loc = 0; loc < maxDepthLoc || mArbitraryDepthLocations.size() > 0; ++loc) {
			if (mSpecificDepthLocations.count(loc) > 0) {
				result.mOrderedDepthAttachmentRefs.push_back(mSpecificDepthLocations[loc]);
			}
			else {
				if (mArbitraryDepthLocations.size() > 0) {
					result.mOrderedDepthAttachmentRefs.push_back(mArbitraryDepthLocations.front());
					mArbitraryDepthLocations.pop();
				}
				else {
					result.mOrderedDepthAttachmentRefs.push_back(unusedAttachmentRef);
				}
			}
		}
		//    resolves => mOrderedResolveAttachmentRefs
		for (uint32_t loc = 0; loc < maxResolveLoc || mArbitraryResolveLocations.size() > 0; ++loc) {
			if (mSpecificResolveLocations.count(loc) > 0) {
				result.mOrderedResolveAttachmentRefs.push_back(mSpecificResolveLocations[loc]);
			}
			else {
				if (mArbitraryResolveLocations.size() > 0) {
					result.mOrderedResolveAttachmentRefs.push_back(mArbitraryResolveLocations.front());
					mArbitraryResolveLocations.pop();
				}
				else {
					result.mOrderedResolveAttachmentRefs.push_back(unusedAttachmentRef);
				}
			}
		}
		//    inputs => mOrderedInputAttachmentRefs
		for (uint32_t loc = 0; loc < maxInputLoc || mArbitraryInputLocations.size() > 0; ++loc) {
			if (mSpecificInputLocations.count(loc) > 0) {
				result.mOrderedInputAttachmentRefs.push_back(mSpecificInputLocations[loc]);
			}
			else {
				if (mArbitraryInputLocations.size() > 0) {
					result.mOrderedInputAttachmentRefs.push_back(mArbitraryInputLocations.front());
					mArbitraryInputLocations.pop();
				}
				else {
					result.mOrderedInputAttachmentRefs.push_back(unusedAttachmentRef);
				}
			}
		}

		assert(result.mOrderedResolveAttachmentRefs.size() == 0
			|| result.mOrderedResolveAttachmentRefs.size() == result.mOrderedColorAttachmentRefs.size());

		// 4. Now we can fill the subpass description
		auto subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(static_cast<uint32_t>(result.mOrderedColorAttachmentRefs.size()))
			.setPColorAttachments(result.mOrderedColorAttachmentRefs.data())
			.setPResolveAttachments(result.mOrderedResolveAttachmentRefs.data())
			.setInputAttachmentCount(static_cast<uint32_t>(result.mOrderedInputAttachmentRefs.size()))
			.setPInputAttachments(result.mOrderedInputAttachmentRefs.data())
			.setPreserveAttachmentCount(static_cast<uint32_t>(result.mOrderedPreserveAttachmentRefs.size()))
			.setPPreserveAttachments(result.mOrderedPreserveAttachmentRefs.data())
	}
}
