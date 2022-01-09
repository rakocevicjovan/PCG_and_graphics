#pragma once

#include "VkInit.h"
#include "VkCmdPool.h"
#include "VkCmdBuffer.h"
#include "VkRenderPass.h"
#include "VkFramebuffer.h"

inline constexpr uint64_t s_to_ns(uint64_t seconds)
{
	return seconds * 1'000'000'000;
}

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

	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

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
		initSyncStructures();
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
		vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU, _device, _surface };

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)	// FIFO_KHR = VSync, good enough for now
			.set_desired_extent(W_WIDTH, W_HEIGHT)	// Window width and height
			.build()
			.value();

		// Store swapchain and its related images
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


	void initSyncStructures()
	{
		// Create synchronization structures - create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCheck(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

		// For the semaphores we don't need any flags
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		vkCheck(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
		vkCheck(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));
	}


	void draw()
	{
		//wait until the GPU has finished rendering the last frame. Timeout of 1 second
		vkCheck(vkWaitForFences(_device, 1, &_renderFence, true, s_to_ns(1)));	// Using this function with 0 timeout can be used to quickly check if it's done
		vkCheck(vkResetFences(_device, 1, &_renderFence));

		// Request image from the swapchain, one second timeout. Signal the _presentSemaphore to synchronize with vkQueueSubmit below (swapchain image must be ready then)
		uint32_t swapchainImageIndex;
		vkCheck(vkAcquireNextImageKHR(_device, _swapchain, s_to_ns(1), _presentSemaphore, nullptr, &swapchainImageIndex));

		vkCheck(vkResetCommandBuffer(_mainCommandBuffer, 0));

		// Naming it cmd for shorter writing
		VkCommandBuffer cmd = _mainCommandBuffer;

		//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;

		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkCheck(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

		//make a clear-color from frame number. This will flash with a 120*pi frame period.
		VkClearValue clearValue;
		static uint64_t frameNumber{ 0u };
		float flash = abs(sin(frameNumber++ / 120.f));
		clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

		// Start the main renderpass
		// We will use the clear color from above, and the framebuffer of the index the swapchain gave us
		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;

		rpInfo.renderPass = _defaultRenderPass;
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = VkExtent2D{ W_WIDTH, W_HEIGHT };
		rpInfo.framebuffer = _framebuffers[swapchainImageIndex];

		//connect clear values
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		/// Rendering goes here (for this renderpass)

		/// Rendering ends here (for this renderpass)

		vkCmdEndRenderPass(cmd);
		vkCheck(vkEndCommandBuffer(cmd));

		// Prepare the submission to the queue.
		// Wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		// Signal the _renderSemaphore, to signal that rendering has finished

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &_presentSemaphore;

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &_renderSemaphore;

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;

		// Submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		vkCheck(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

		// this will put the image we just rendered into the visible window.
		// we want to wait on the _renderSemaphore for that,
		// as it's necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &_renderSemaphore;

		presentInfo.pImageIndices = &swapchainImageIndex;

		vkCheck(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

		//increase the number of frames drawn
		frameNumber++;
	}


	// Destroy
	void cleanup()
	{
		if (_isInitialized)
		{
			// VkQueue, much like the VkPhysicalDevice, is a handle to an existing thing as opposed to something we created, and can't be destroyed here
			
			// Individual buffers allocated from this command pool will be destroyed with it (can't be destroyed individually)
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