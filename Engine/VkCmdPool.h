#pragma once

#include "VkTypes.h"


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

	//the command pool will be one that can submit graphics commands
	// we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = CreateCommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	vkCheck(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	return commandPool;
}