#pragma once

#include "VkTypes.h"


VkCommandBufferAllocateInfo CreateCmdBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;	// secondary buffers are for subcommands to the primary bufferand used in advanced multithreading scenarios
	return info;
}


VkCommandBuffer create(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdAllocInfo = CreateCmdBufferAllocateInfo(commandPool);

	vkCheck(vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmdBuffer));

	return cmdBuffer;
}