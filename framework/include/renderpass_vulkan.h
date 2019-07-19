#pragma once

namespace cgb
{
	class renderpass
	{
		static renderpass create(std::initializer_list<attachment> pAttachments);

		std::vector<vk::AttachmentDescription> mAttachmentDescriptions;
		std::vector<vk::AttachmentReference> mOrderedColorAttachmentRefs;
		std::vector<vk::AttachmentReference> mOrderedDepthAttachmentRefs;
		std::vector<vk::AttachmentReference> mOrderedResolveAttachmentRefs;
		std::vector<vk::AttachmentReference> mOrderedInputAttachmentRefs;
		std::vector<vk::AttachmentReference> mOrderedPreserveAttachmentRefs;
		std::vector<vk::SubpassDescription> mSubpasses;
		std::vector<vk::SubpassDependency> mSubpassDependencies;
		vk::UniqueRenderPass mRenderPass;
	};

	extern bool are_compatible(const renderpass& first, const renderpass& second);

	// Helper function:
	template <typename V>
	void insert_at_first_unused_location_or_push_back(V& pCollection, uint32_t pAttachment, vk::ImageLayout pImageLayout)
	{
		auto newElement = vk::AttachmentReference{}
			.setAttachment(pAttachment)
			.setLayout(pImageLayout);

		size_t n = pCollection.size();
		for (size_t i = 0; i < n; ++i) {
			if (pCollection[i].attachment == VK_ATTACHMENT_UNUSED) {
				// Found a place to insert
				pCollection[i] = std::move(newElement);
				return;
			}
		}
		// Couldn't find a spot with an unused attachment
		pCollection.push_back(std::move(newElement));
	}

	// Helper function:
	template <typename V>
	std::optional<vk::AttachmentReference> insert_at_location_and_possibly_get_already_existing_element(V& pCollection, uint32_t pTargetLocation, uint32_t pAttachment, vk::ImageLayout pImageLayout)
	{
		auto newElement = vk::AttachmentReference{}
			.setAttachment(pAttachment)
			.setLayout(pImageLayout);

		size_t l = static_cast<size_t>(pTargetLocation);
		// Fill up with unused attachments if required:
		while (pCollection.size() <= l) {
			pCollection.push_back(vk::AttachmentReference{}
				.setAttachment(VK_ATTACHMENT_UNUSED)
			);
		}
		// Set at the requested location:
		auto existingElement = pCollection[l];
		pCollection[l] = newElement;
		if (existingElement.attachment == VK_ATTACHMENT_UNUSED) {
			return {};
		}
		else {
			return existingElement;
		}
	}
}