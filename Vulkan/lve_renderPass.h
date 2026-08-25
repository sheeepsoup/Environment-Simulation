#pragma once
#include<vulkan/vulkan.h>
#include<iostream>
namespace lve {
	class LveRenderPass {
	public:
		VkRenderPass &getRenderPass() { return renderPass; };
		void createRenderPass(VkSurfaceFormatKHR surfaceFormat, VkFormat depthFormat, VkDevice device);
		void clean(VkDevice device);
	private:
		VkAttachmentDescription colorAttachment{};//颜色缓冲区附件
		VkAttachmentReference colorAttachmentRef{};//颜色附件
		VkSubpassDescription subpass{};//子附件
		VkSubpassDependency dependency{};//子通道依赖
		VkRenderPass renderPass;//渲染通道
	};
}