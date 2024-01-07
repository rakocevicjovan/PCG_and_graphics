#pragma once

#include "rendering/vulkan/VkTypes.h"

// A renderpass renders into a framebuffer, which links to the images being rendered to
// Renderpass contains N subpasses, at least one. On mobile GPUs this is more important than on desktop so currently we can just default to 1


VkAttachmentDescription CreateAttachmentDescription(VkFormat format,
	VkSampleCountFlagBits sampleCountFlag = VK_SAMPLE_COUNT_1_BIT,
	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE)
{
	VkAttachmentDescription desc{};

	desc.format = format;
	desc.samples = sampleCountFlag;
	desc.loadOp = loadOp;
	desc.storeOp = storeOp;
	desc.stencilLoadOp = stencilLoadOp;
	desc.stencilStoreOp = stencilStoreOp;
	desc.initialLayout = initialLayout;
	desc.finalLayout = finalLayout;

	return desc;
}


VkSubpassDescription CreateSubpassDescription(const VkAttachmentReference& attachmentRefs, 
	uint32_t attachmentCount = 1u,
	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
{
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = bindPoint;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentRefs;
	return subpass;
}


VkRenderPass CreateRenderPass(VkDevice device, VkFormat format)
{
	// Create color attachment description
	VkAttachmentDescription colorAttachmentDesc = CreateAttachmentDescription(format);

	// Attachment number will index into the pAttachments array in the parent renderpass itself
	VkAttachmentReference colorAttachmentRef{ 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	// Create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = CreateSubpassDescription(colorAttachmentRef);

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachmentDesc;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkRenderPass renderPass;
	vkCheck(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	return renderPass;
}