#pragma once

#include <VkBootstrap.h>
#include "VkTypes.h"


inline static void vkCheck(VkResult x)
{
	if (x)
	{
		assert(false && "Detected Vulkan error");
	}
}


struct VulkanHandles
{
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
	VkPhysicalDevice _chosenGPU;
	VkDevice _device;		// Vulkan device for commands
	VkSurfaceKHR _surface;	// Vulkan window surface

};


namespace vkinit
{
	void createVkHandles()
	{
		VulkanHandles handles;

		vkb::InstanceBuilder builder;
		
		vkb::detail::Result built = builder.
			set_app_name("Aeolian").
			request_validation_layers(true).
			require_api_version(1, 1, 0).
			use_default_debug_messenger().
			build();

		if (!built.has_value())
		{
			assert(false && "VkBootstrap instance builder error");
		}

		auto vkbInstance = built.value();
		
		handles._instance = vkbInstance.instance;
		handles._debugMessenger = vkbInstance.debug_messenger;

		// Use vkbootstrap to select a GPU. We want a GPU that can write to the SDL surface and supports Vulkan 1.1
		vkb::PhysicalDeviceSelector selector{ built.value() };
		
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 1)
			.set_surface(VkSurfaceKHR{})	//figure out how to pass hwnd here
			.select()
			.value();

		handles._chosenGPU = physicalDevice.physical_device;

		//create the final Vulkan device
		vkb::DeviceBuilder deviceBuilder{ physicalDevice };
		vkb::Device vkbDevice = deviceBuilder.build().value();

		
		handles._device = vkbDevice.device;	// Get the VkDevice handle used in the rest of a Vulkan application
	}
}