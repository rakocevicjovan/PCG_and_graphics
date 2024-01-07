#pragma once

#include "rendering/vulkan/VkTypes.h"

class VkShader
{
public:

	// SPIRV expects data to be uint32_t type so provide that. I assume alignment must be followed as well so beware reinterpret casting a char* with no size check
	bool loadModule(VkDevice device, uint32_t* data, uint32_t size, VkShaderModule* outShaderModule)
	{
		// Create a new shader module, using the buffer we loaded
		VkShaderModuleCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;

		// codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
		createInfo.codeSize = size;
		createInfo.pCode = data;

		//check that the creation goes well.
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			return false;
		}
		*outShaderModule = shaderModule;
		return true;
	}
};