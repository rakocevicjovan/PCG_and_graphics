#pragma once


#include <vulkan/vulkan.h>


inline static void vkCheck(VkResult x)
{
	if (x)
	{
		assert(false && "Detected Vulkan error");
	}
}