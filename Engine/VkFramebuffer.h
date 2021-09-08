#pragma once

#include "VkTypes.h"
#include <vector>


VkFramebufferCreateInfo CreateFramebufferDesc(VkRenderPass renderPass, uint32_t width, uint32_t height, uint32_t attachmentCount = 1u)
{
	VkFramebufferCreateInfo desc{};
	desc.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	desc.pNext = nullptr;
	desc.renderPass = renderPass;
	desc.attachmentCount = attachmentCount;
	desc.width = width;
	desc.height = height;
	desc.layers = 1;

	return desc;
}


std::vector<VkFramebuffer> CreateFramebuffers(VkDevice device, VkRenderPass renderPass, uint32_t width, uint32_t height, VkImageView* attachmentArr, uint32_t attachmentCount = 1u)
{
	std::vector<VkFramebuffer> framebuffers(attachmentCount);

	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo framebufferDesc = CreateFramebufferDesc(renderPass, width, height);

	// create framebuffers for each of the swapchain image views
	for (int i = 0; i < attachmentCount; ++i)
	{
		framebufferDesc.pAttachments = &attachmentArr[i];
		vkCheck(vkCreateFramebuffer(device, &framebufferDesc, nullptr, &framebuffers[i]));
	}

	return framebuffers;
}