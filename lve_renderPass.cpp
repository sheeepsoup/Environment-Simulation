#include"lve_renderPass.h"
namespace lve {
	void LveRenderPass::createRenderPass(VkSurfaceFormatKHR surfaceFormat, VkFormat depthFormat, VkDevice device) {
		//深度缓冲附件
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		//深度缓冲附件引用
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		//颜色缓冲区附件
		colorAttachment.format = surfaceFormat.format;//图像格式
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//采样数:1个[去掉多重采样]
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//每一帧从空白开始
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//绘画完毕后保存结果
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//不关心模板缓冲旧数据
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//不保存模板缓冲结果
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//开始时的内存布局
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//结束时的内存布局

		//渲染流程
		colorAttachmentRef.attachment = 0;//附件索引,指向为第0个
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//附件当作颜色附件来用

		//子过程
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//表示子附件在图形渲染管线用
		subpass.colorAttachmentCount = 1;//表示使用一个颜色附件
		subpass.pColorAttachments = &colorAttachmentRef;//对应的附件


		//子通道
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;//外部子通道
		dependency.dstSubpass = 0;//目标子通道
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;//源阶段
		dependency.srcAccessMask = 0;//源访问掩码
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;//目标阶段
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;//目标访问掩码
		subpass.pDepthStencilAttachment = &depthAttachmentRef;//深度缓冲

		//创建渲染通道对象
		VkAttachmentDescription attachments[] = {
			colorAttachment,
			depthAttachment
		};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;//通道有1个子过程
		renderPassInfo.pSubpasses = &subpass;//对应子过程
		renderPassInfo.dependencyCount = 1;//通道有1个子通道依赖
		renderPassInfo.pDependencies = &dependency;//对应子通道依赖
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {//创建通道
			throw std::runtime_error("failed to create render pass!");
		}

	}
	void LveRenderPass::clean(VkDevice device) {
		vkDestroyRenderPass(device, renderPass, nullptr);
	}
}