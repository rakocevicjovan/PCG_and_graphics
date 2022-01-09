#pragma once

#include "VkTypes.h"

// One vulkan pool (with n buffers) per thread is required to record commands on multiple threads in parallel, vkQueueSubmit however is not thread safe

VkCommandPoolCreateInfo CreateCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0)
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;
	return info;
}


VkCommandPool CreatePool(VkDevice device, uint32_t graphicsQueueFamily)
{
	VkCommandPool commandPool;

	// The pool will allow for resetting of individual command buffers - although resetting them all via the pool is faster
	VkCommandPoolCreateInfo commandPoolInfo = CreateCommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	vkCheck(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	return commandPool;
}