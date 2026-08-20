#include"lve_swapChain.h"
namespace lve {
	void LveSwapChain::createSwapChain(LveDevice& device, LveWindows& win) {
		//制作交换链
	//检查是否支持交换链
	//1. 查询交换链支持细节
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), win.getSurface(), &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), win.getSurface(), &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), win.getSurface(), &formatCount, formats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), win.getSurface(), &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), win.getSurface(), &presentModeCount, presentModes.data());

		//2. 选择表面格式
		surfaceFormat = formats[0];
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = format;
				break;
			}
		}

		//3. 选择呈现模式（FIFO保证可用，MAILBOX是三重缓冲）
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& mode : presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = mode;
				break;
			}
		}

		//4. 选择交换范围（图像大小）

		if (capabilities.currentExtent.width != UINT32_MAX) {
			extent = capabilities.currentExtent;
		}
		else {
			extent = { unsigned int(win.get_heigh_width().w), unsigned int(win.get_heigh_width().h) };
		}

		//5. 确定图像数量（至少比最小值多1张）
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			imageCount = capabilities.maxImageCount;
		}

		//6. 创建交换链
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = win.getSurface();
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = surfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		lve::LveDevice::QueueFamilyIndices indices = device.finQueueFamiles(device.getPhysicalDevice(), win);
		queueFamilyIndices[0] = (uint32_t)indices.graphicsFamily;
		queueFamilyIndices[1] = (uint32_t)indices.presentFamily;

		if (indices.graphicsFamily != indices.presentFamily) {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		swapchainCreateInfo.preTransform = capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;


		if (vkCreateSwapchainKHR(device.getDevice(), &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("创建交换链失败！");
		}

		//7. 获取交换链图像

		vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &imageCount, swapchainImages.data());

		//8. 创建图像视图

		swapchainImageViews.resize(swapchainImages.size());

		for (size_t i = 0; i < swapchainImages.size(); i++) {
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = swapchainImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = surfaceFormat.format;
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.getDevice(), &viewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("create swapchain image view error!!!");
			}
		}
	}
	void LveSwapChain::recreateSwapChain(LveDevice& device, LveWindows& win, VkRenderPass renderPass) {
		vkDeviceWaitIdle(device.getDevice());//等待设备空闲，确保没有命令在使用旧的交换链
		//清理旧的交换链资源
		cleanupSwapChain(device.getDevice());
		//重新创建交换链
		createSwapChain(device, win);
		createDepthResources(device);
		createFrameBuffer(device, renderPass);
	}

	void LveSwapChain::createFrameBuffer(LveDevice& device, VkRenderPass renderPass) {
		swapChainFramebuffers.resize(swapchainImages.size());//预设大小
		for (size_t i = 0; i < swapchainImageViews.size(); i++) {

			VkFramebufferCreateInfo framebufferInfo{};//帧缓冲区信息
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;//帧缓冲区信息类型
			framebufferInfo.renderPass = renderPass;//渲染通道
			VkImageView attachments[] = {
				swapchainImageViews[i],
				depthImageView
			};
			framebufferInfo.attachmentCount = 2;//附件数量
			framebufferInfo.pAttachments = attachments;//附件
			framebufferInfo.width = extent.width;//帧缓冲区宽度
			framebufferInfo.height = extent.height;//帧缓冲区高度
			framebufferInfo.layers = 1;//帧缓冲区层数

			if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {//创建帧缓冲区,结果保存在swapChainFramebuffers[i]中
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	};

	void LveSwapChain::cleanupSwapChain(VkDevice device) {//this function is used to clean up the swap chain resources[prticualy used in recraete]
		for (VkFramebuffer framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(
				device,
				framebuffer,
				nullptr
			);
		}
		swapChainFramebuffers.clear();

		if (depthImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device, depthImageView, nullptr);
			depthImageView = VK_NULL_HANDLE;
		}

		if (depthImage != VK_NULL_HANDLE) {
			vkDestroyImage(device, depthImage, nullptr);
			depthImage = VK_NULL_HANDLE;
		}

		if (depthImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, depthImageMemory, nullptr);
			depthImageMemory = VK_NULL_HANDLE;
		}

		for (VkImageView imageView : swapchainImageViews) {
			vkDestroyImageView(
				device,
				imageView,
				nullptr
			);
		}
		swapchainImageViews.clear();

		if (swapchain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(
				device,
				swapchain,
				nullptr
			);

			swapchain = VK_NULL_HANDLE;
		}

		swapchainImages.clear();
	}
	
	void LveSwapChain::createDepthResources(LveDevice& lveDevice) {
		VkFormat depthFormat = lveDevice.findDepthFormat();//查询深度缓冲格式
		lveDevice.createImage(extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = lveDevice.createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		
	}

}

