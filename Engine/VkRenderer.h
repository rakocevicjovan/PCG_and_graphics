#pragma once

#include "VkInit.h"
#include "VkCmdPool.h"
#include "VkCmdBuffer.h"
#include "VkRenderPass.h"
#include "VkFramebuffer.h"


class VkRenderer
{
	// Vulkan handles
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;
	VkPhysicalDevice _chosenGPU;
	VkDevice _device;		// Vulkan device for commands
	VkSurfaceKHR _surface;	// Vulkan window surface

	// Swapchain
	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;	// image format expected by the windowing system
	std::vector<VkImage> _swapchainImages;	//array of images from the swapchain
	std::vector<VkImageView> _swapchainImageViews;	//array of image-views from the swapchain

	// Queue, pool, buffer
	VkQueue _graphicsQueue; //queue we will submit to
	uint32_t _graphicsQueueFamily; //family of that queue

	VkCommandPool _commandPool; //the command pool for our commands
	VkCommandBuffer _mainCommandBuffer; //the buffer we will record into

	VkRenderPass _defaultRenderPass;
	std::vector<VkFramebuffer> _framebuffers;

	static constexpr uint32_t W_WIDTH = 2560u;
	static constexpr uint32_t W_HEIGHT = 1440u;

	bool _isInitialized{ false };


	void init()
	{
		initVulkan();
		initSwapchain();
		initCommands();
		initDefaultRenderPass();
		initFrameBuffers();
		_isInitialized = true;	// If we got here, it's good. Above would throw/assert otherwise.
	}


	void initVulkan()
	{
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

		_instance = vkbInstance.instance;
		_debugMessenger = vkbInstance.debug_messenger;

		// Use vkbootstrap to select a GPU. We want a GPU that can write to the SDL surface and supports Vulkan 1.1
		vkb::PhysicalDeviceSelector selector{ built.value() };

		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 1)
			.set_surface(VkSurfaceKHR{})	//figure out how to pass hwnd here
			.select()
			.value();

		_chosenGPU = physicalDevice.physical_device;

		//create the final Vulkan device
		vkb::DeviceBuilder deviceBuilder{ physicalDevice };
		vkb::Device vkbDevice = deviceBuilder.build().value();

		_device = vkbDevice.device;	// Get the VkDevice handle used in the rest of a Vulkan application

		// use vkbootstrap to get a Graphics queue
		_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	}


	void initSwapchain()
	{
		vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU,_device,_surface };

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			//use vsync present mode
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)	// FIFO_KHR = VSync, good enough for now
			.set_desired_extent(W_WIDTH, W_HEIGHT)	// Window width and height
			.build()
			.value();

		//store swapchain and its related images
		_swapchain = vkbSwapchain.swapchain;
		_swapchainImages = vkbSwapchain.get_images().value();
		_swapchainImageViews = vkbSwapchain.get_image_views().value();

		_swapchainImageFormat = vkbSwapchain.image_format;
	}

	
	void initCommands()
	{
		CreatePool(_device, _graphicsQueueFamily);
	}


	void initDefaultRenderPass()
	{
		// the renderpass will use this color attachment.
		// the attachment will have the format needed by the swapchain
		// 1 sample, we won't be doing MSAA
		// we Clear when this attachment is loaded
		// we keep the attachment stored when the renderpass ends
		// we don't care about stencil
		// we don't know or care about the starting layout of the attachment
		// after the renderpass ends, the image has to be on a layout ready for display
		_defaultRenderPass = CreateRenderPass(_device, _swapchainImageFormat);
	}


	void initFrameBuffers()
	{
		_framebuffers = CreateFramebuffers(_device, _defaultRenderPass, W_WIDTH, W_HEIGHT, _swapchainImageViews.data(), _swapchainImageViews.size());
	}


	// Destroy
	void cleanup()
	{
		if (_isInitialized)
		{
			// VkQueue, much like the VkPhysicalDevice, is a handle to an existing thing as opposed to something we created, and can't be destroyed here
			// Individual buffers allocated from this pool will be destroyed with it (can't destroy individually)
			vkDestroyCommandPool(_device, _commandPool, nullptr);	

			vkDestroySwapchainKHR(_device, _swapchain, nullptr);

			// Render pass with frame buffers
			vkDestroyRenderPass(_device, _defaultRenderPass, nullptr);

			for (int i = 0; i < _framebuffers.size(); i++)
			{
				vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
				vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			}

			//destroy swapchain resources
			for (int i = 0; i < _swapchainImageViews.size(); i++)
			{
				vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			}

			vkDestroyDevice(_device, nullptr);
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
			vkDestroyInstance(_instance, nullptr);
		}
	}
};