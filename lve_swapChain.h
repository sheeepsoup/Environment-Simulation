#pragma once
#include<vector>
#include<vulkan/vulkan.h>
#include"lve_device.h"
#include"lve_windows.h"
#include"lve_renderPass.h"
namespace lve {
	class LveSwapChain {
	public:
		void createSwapChain(LveDevice &device, LveWindows &win);
		void recreateSwapChain(LveDevice& device, LveWindows& win, VkRenderPass renderPass);//recreate swap chain
		void createFrameBuffer(LveDevice &device, VkRenderPass renderPass);
		int getSwapChainImageCount() { return swapchainImages.size(); };
		void cleanupSwapChain(VkDevice device);
		void createDepthResources(LveDevice& lveDevice);//创建深度缓冲
		uint32_t getQueueFamilyIndices_what(size_t index) { return queueFamilyIndices[index]; };
		VkExtent2D getSwapChainExtent() { return extent; };
		VkSwapchainKHR getSwapChain() { return swapchain; };
		VkFramebuffer getSwapChainFrameBuffer(size_t index) { return swapChainFramebuffers[index]; };
		VkSurfaceFormatKHR getSwapChainSurfaceFormat() { return surfaceFormat; };

	private:
		VkSwapchainKHR swapchain;//交换链
		VkSurfaceFormatKHR surfaceFormat;//表面格式
		VkExtent2D extent;//交换链范围
		std::vector<VkImage> swapchainImages;//交换链有几个图像
		std::vector<VkImageView> swapchainImageViews;//交换链的图像视图

		VkImage depthImage = VK_NULL_HANDLE;//深度缓冲图
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;//深度缓冲对应内存
		VkImageView depthImageView = VK_NULL_HANDLE;//内存视图

		std::vector<VkFramebuffer> swapChainFramebuffers;//帧缓冲区
		uint32_t queueFamilyIndices[2];//队列簇索引
	};
}